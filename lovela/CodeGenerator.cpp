#include "pch.h"
#include "CodeGenerator.h"

std::map<Node::Type, CodeGenerator::Visitor> CodeGenerator::visitors
{
	{Node::Type::FunctionDeclaration, &CodeGenerator::FunctionDeclaration}
};

std::map<Node::Type, CodeGenerator::Visitor> CodeGenerator::internalVisitors
{
	{Node::Type::Expression, &CodeGenerator::Expression},
	{Node::Type::ExpressionInput, &CodeGenerator::ExpressionInput},
	{Node::Type::FunctionCall, &CodeGenerator::FunctionCall},
	{Node::Type::BinaryOperation, &CodeGenerator::BinaryOperation},
	{Node::Type::Literal, &CodeGenerator::Literal},
	{Node::Type::Tuple, &CodeGenerator::Tuple},
	{Node::Type::VariableReference, &CodeGenerator::VariableReference},
};

const TypeSpec CodeGenerator::NoneType{ .name = L"lovela::None" };
const TypeSpec CodeGenerator::VoidType{ .name = L"void" };
const TypeSpec CodeGenerator::VoidPtrType{ .name = L"void*" };

CodeGenerator::CodeGenerator(std::wostream& stream) : stream(stream)
{
}

void CodeGenerator::Visit(Node& node)
{
	auto iter = visitors.find(node.type);
	if (iter != visitors.end())
	{
		Context context;
		iter->second(this, node, context);
	}
}

void CodeGenerator::Visit(Node& node, Context& context)
{
	auto iter = internalVisitors.find(node.type);
	if (iter != internalVisitors.end())
	{
		iter->second(this, node, context);
	}
}

void CodeGenerator::BeginScope()
{
	stream << Indent() << "{\n";
	indent += '\t';
}

void CodeGenerator::EndScope()
{
	if (indent.empty())
	{
		throw std::exception("Scope begin and end mismatch.");
	}

	indent.resize(indent.length() - 1);
	stream << Indent() << "}\n";
}

void CodeGenerator::FunctionDeclaration(Node& node, Context& context)
{
	if (node.value.empty())
	{
		MainFunctionDeclaration(node, context);
		return;
	}
	else if (node.imported)
	{
		ImportedFunctionDeclaration(node, context);
	}

	auto inType = node.inType;
	auto outType = node.outType;
	std::vector<std::wstring> templateParameters;
	std::vector<std::pair<std::wstring, std::wstring>> parameters;

	if (outType.Any())
	{
		outType.name = L"Out";
		templateParameters.push_back(outType.name);
	}
	else if (node.outType.None())
	{
		outType = NoneType;
	}
	else
	{
		outType.name = TypeName(outType.name);
	}

	if (inType.Any())
	{
		parameters.emplace_back(std::make_pair(L"In", L"in"));
		templateParameters.emplace_back(L"In");
	}
	else if (inType.None())
	{
		parameters.emplace_back(std::make_pair(NoneType.name, L"in"));
	}
	else
	{
		parameters.emplace_back(std::make_pair(TypeName(inType.name), L"in"));
	}

	int index = 0;
	for (auto& parameter : node.parameters)
	{
		index++;
		const auto name = ParameterName(parameter->name, index);
		const auto type = TypeName(parameter->type.name, index);

		if (parameter->type.Any())
		{
			templateParameters.push_back(type);
		}

		parameters.emplace_back(std::make_pair(type, name));
	}

	if (!templateParameters.empty())
	{
		stream << Indent() << "template <";

		index = 0;
		for (auto& param : templateParameters)
		{
			stream << (index++ ? ", " : "");
			stream << "typename " << param;
		}

		stream << ">\n";
	}

	stream << Indent() << outType.name << ' ' << FunctionName(node.value) << "(lovela::context& context";

	for (auto& parameter : parameters)
	{
		stream << ", " << parameter.first << ' ' << parameter.second;
	}

	stream << ')';

	if (node.imported)
	{
		ImportedFunctionBody(node, context, parameters);
	}
	else
	{
		FunctionBody(node, context);
	}

	stream << '\n';

	// Generate the exported function

	if (node.exported)
	{
		ExportedFunctionDeclaration(node, context);
	}
}

void CodeGenerator::MainFunctionDeclaration(Node& node, Context& context)
{
	if (!node.outType.None())
	{
		errors.emplace_back(L"Warning: The main function out type wasn't None. The parser should set that.");
		node.outType.SetNone();
	}

	stream << NoneType.name << ' ' << "lovela::main(lovela::context& context, " << NoneType.name << " in)";
	FunctionBody(node, context);
	stream << '\n';
}

bool CodeGenerator::CheckExportType(TypeSpec& type)
{
	if (type.Any())
	{
		type = VoidPtrType;
	}
	else if (!ConvertPrimitiveType(type.name))
	{
		errors.emplace_back(L"Error: Exported functions must have primitive in, out and parameter types. Unsupported type: " + type.name);
		return false;
	}

	return true;
}

bool CodeGenerator::ConvertPrimitiveType(std::wstring& name)
{
	static std::wregex regex(LR"(#(\.|\+)?(\d\d?)(#?)(#?))");
	std::wsmatch match;
	if (!std::regex_match(name, match, regex))
	{
		return false;
	}

	std::wstring exportName = match[1].str() == L"." ? L"float" : (match[1].str() == L"+" ? L"uint" : L"int");
	exportName += match[2].str() + L"_t";
	exportName += match[3].str() == L"#" ? L"*" : L"";
	exportName += match[4].str() == L"#" ? L"*" : L"";
	name = exportName;

	return true;
}

std::wstring CodeGenerator::TypeName(const std::wstring& name)
{
	std::wstring typeName = name;
	if (ConvertPrimitiveType(typeName))
	{
		return typeName;
	}

	return L"t_" + name;
}

std::wstring CodeGenerator::TypeName(const std::wstring& name, int index)
{
	if (name.empty())
	{
		return L"Param" + to_wstring(index);
	}
	else
	{
		return TypeName(name);
	}
}

std::wstring CodeGenerator::ParameterName(const std::wstring& name)
{
	return L"p_" + name;
}

std::wstring CodeGenerator::ParameterName(const std::wstring& name, int index)
{
	if (name.empty())
	{
		return L"param" + to_wstring(index);
	}
	else
	{
		return ParameterName(name);
	}
}

std::wstring CodeGenerator::FunctionName(const std::wstring& name)
{
	return L"f_" + name;
}

void CodeGenerator::ExportedFunctionDeclaration(Node& node, Context&)
{
	auto inType = node.inType;
	auto outType = node.outType;

	std::vector<std::pair<std::wstring, std::wstring>> parameters;

	// Verify and convert the input type

	if (inType.None())
	{
	}
	else if (CheckExportType(inType))
	{
		parameters.emplace_back(std::make_pair(inType.name, L"in"));
	}
	else
	{
		return;
	}

	// Verify and convert the output type

	if (outType.None())
	{
		outType = VoidType;
	}
	else if (!CheckExportType(outType))
	{
		return;
	}

	// Verify and convert the parameter types

	int index = 0;
	for (auto& parameter : node.parameters)
	{
		index++;
		const auto name = ParameterName(parameter->name, index);
		auto type = parameter->type;

		if (!CheckExportType(type))
		{
			return;
		}

		parameters.emplace_back(std::make_pair(type.name, name));
	}

	// Make the function signature

	std::wstringstream ss;
	ss << outType.name << ' ' << node.value << '(';

	index = 0;
	for (auto& parameter : parameters)
	{
		ss << (index++ ? ", " : "") << parameter.first << ' ' << parameter.second;
	}

	ss << ')';

	auto signature = ss.str();

	// Store for export declarations

	exports.push_back(signature);

	// Define the exported function wrapper

	stream << signature << '\n';

	BeginScope();

	stream << Indent() << "lovela::context context;\n";

	if (inType.None())
	{
		stream << Indent() << NoneType.name << " in;\n";
	}

	// Call the actual function

	stream << Indent() << (node.outType.None() ? "" : "return ") << FunctionName(node.value) << "(context";

	if (inType.None())
	{
		stream << ", in";
	}

	for (auto& parameter : parameters)
	{
		stream << ", " << parameter.second;
	}

	stream << ");\n";

	EndScope();

	stream << '\n';
}

void CodeGenerator::ImportedFunctionDeclaration(Node& node, Context&)
{
	auto inType = node.inType;
	auto outType = node.outType;

	std::vector<std::pair<std::wstring, std::wstring>> parameters;

	// Verify and convert the input type

	if (inType.None())
	{
	}
	else if (CheckExportType(inType))
	{
		parameters.emplace_back(std::make_pair(inType.name, L"in"));
	}
	else
	{
		return;
	}

	// Verify and convert the output type

	if (outType.None())
	{
		outType = VoidType;
	}
	else if (!CheckExportType(outType))
	{
		return;
	}

	// Verify and convert the parameter types

	int index = 0;
	for (auto& parameter : node.parameters)
	{
		index++;
		const auto name = ParameterName(parameter->name, index);
		auto type = parameter->type;

		if (!CheckExportType(type))
		{
			return;
		}

		parameters.emplace_back(std::make_pair(type.name, name));
	}

	// Make the function signature

	std::wstringstream ss;
	ss << outType.name << ' ' << node.value << '(';

	index = 0;
	for (auto& parameter : parameters)
	{
		ss << (index++ ? ", " : "") << parameter.first << ' ' << parameter.second;
	}

	ss << ')';

	auto signature = ss.str();

	// Declare import

	stream << "extern \"C\" " << signature << ";\n";

	stream << '\n';
}

void CodeGenerator::FunctionBody(Node& node, Context& context)
{
	if (node.left)
	{
		stream << '\n';

		BeginScope();

		stream << Indent() << "context;\n";

		// Make an indexed reference to the input object and avoid a warning if it's unreferenced.
		stream << Indent() << "auto& " << LocalVar << ++context.variableIndex << " = in; " << LocalVar << context.variableIndex << ";\n";

		context.assignVariable = true;
		Visit(*node.left, context);

		if (node.outType.None())
		{
			stream << Indent() << "return {};\n";
		}
		else
		{
			stream << Indent() << "return " << LocalVar << context.variableIndex << ";\n";
		}

		EndScope();
	}
	else
	{
		stream << ";\n";
	}
}

void CodeGenerator::ImportedFunctionBody(Node& node, Context&, const std::vector<std::pair<std::wstring, std::wstring>>& parameters)
{
	stream << '\n';

	BeginScope();

	stream << Indent();

	if (!node.outType.None())
	{
		stream << "return ";
	}

	stream << node.value << '(';

	int index = 0;
	for (auto& parameter : parameters)
	{
		stream << (index++ ? ", " : "");
		stream << parameter.second;
	}

	stream << ");\n";

	if (node.outType.None())
	{
		stream << "return {};\n";
	}

	EndScope();
}

void CodeGenerator::Expression(Node& node, Context& context)
{
	if (node.left)
	{
		BeginAssign(context);
		Visit(*node.left, context);
		EndAssign(context);
	}

	if (node.right)
	{
		Visit(*node.right, context);
	}
}

void CodeGenerator::ExpressionInput(Node&, Context& context)
{
	// The input of an expression is the output of the previous expression.
	stream << LocalVar << (context.variableIndex - 1) << ' ';
}

void CodeGenerator::FunctionCall(Node& node, Context& context)
{
	const auto reset = BeginAssign(context, false);

	stream << FunctionName(node.value) << "(context";

	if (node.left)
	{
		stream << ", ";
		Visit(*node.left, context);
	}

	if (node.right)
	{
		stream << ", ";
		Visit(*node.right, context);
	}

	stream << ')';

	EndAssign(context, reset);
}

void CodeGenerator::BinaryOperation(Node& node, Context& context)
{
	const bool reset = BeginAssign(context, false);

	if (node.left)
	{
		Visit(*node.left, context);
	}
	else
	{
		errors.emplace_back(L"Left node missing in binary operation.");
	}

	stream << node.value << ' ';

	if (node.right)
	{
		Visit(*node.right, context);
	}
	else
	{
		errors.emplace_back(L"Right node missing in binary operation.");
	}

	EndAssign(context, reset);
}

void CodeGenerator::Literal(Node& node, Context& context)
{
	BeginAssign(context);
	stream << (node.token.type == Token::Type::LiteralString ? double_quote(node.value) : node.value);
	EndAssign(context);
}

void CodeGenerator::Tuple(Node& node, Context& context)
{
	const bool hasLeft = !!node.left;
	if (hasLeft)
	{
		Visit(*node.left, context);
	}

	if (node.right)
	{
		if (hasLeft)
		{
			stream << ", ";
		}

		Visit(*node.right, context);
	}
}

void CodeGenerator::VariableReference(Node& node, Context&)
{
	stream << ParameterName(node.value);
}

void CodeGenerator::BeginAssign(Context& context)
{
	if (context.assignVariable)
	{
		stream << Indent() << "const auto " << LocalVar << ++context.variableIndex << " = ";
	}
}

bool CodeGenerator::BeginAssign(Context& context, bool set)
{
	BeginAssign(context);

	const auto reset = context.assignVariable;
	context.assignVariable = set;
	return reset;
}

void CodeGenerator::EndAssign(Context& context)
{
	if (context.assignVariable)
	{
		stream << "; " << LocalVar << context.variableIndex << ";\n";
	}
}

void CodeGenerator::EndAssign(Context& context, bool reset)
{
	context.assignVariable = reset;

	EndAssign(context);
}

void CodeGenerator::GenerateLibraryHeaderFile(std::wostream& file)
{
	CodeGenerator::BeginLibraryHeaderFile(file);

	for (auto& signature : GetExports())
	{
		file << signature << ";\n";
	}

	CodeGenerator::EndLibraryHeaderFile(file);
}

void CodeGenerator::BeginProgramSourceFile(std::wostream& file)
{
	file << "#include \"lovela-program.h\"\n\n";
}

void CodeGenerator::EndProgramSourceFile(std::wostream&)
{
}

void CodeGenerator::BeginLibraryHeaderFile(std::wostream& file)
{
	file << "#ifndef LOVELA_EXPORTS\n#define LOVELA_EXPORTS\n\n";
}

void CodeGenerator::EndLibraryHeaderFile(std::wostream& file)
{
	file << "\n#endif\n";
}
