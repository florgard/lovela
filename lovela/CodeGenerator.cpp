#include "pch.h"
#include "CodeGenerator.h"
#include "StandardCDeclarations.h"
#include "StandardCppDeclarations.h"

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
	else if (node.api.Is(Api::Import))
	{
		ImportedFunctionDeclaration(node, context);
	}

	auto inType = node.inType;
	auto outType = node.outType;
	std::vector<std::wstring> templateParameters;
	std::vector<std::pair<std::wstring, std::wstring>> parameters;

	if (outType.Any())
	{
		outType.name = L"auto";
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

	if (node.api.Is(Api::Import))
	{
		ImportedFunctionBody(node, context, parameters);
	}
	else
	{
		FunctionBody(node, context);
	}

	stream << '\n';

	// Generate the exported function

	if (node.api.Is(Api::Export))
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
	static std::map<std::wstring, std::wstring> types{
		{L"#8#", L"l_cstr"},
	};

	if (types.contains(name))
	{
		name = types.at(name);
		return true;
	}

	static std::wregex regex(LR"(#(\+)?(\d\d?)(#?)(#?))");
	std::wsmatch match;
	if (!std::regex_match(name, match, regex))
	{
		return false;
	}

	std::wstring exportName = L"l_";
	exportName += match[1].str() == L"." ? L'f' : (match[1].str() == L"+" ? L'u' : L'i');
	exportName += match[2].str();
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

	// Store export declaration

	std::wostringstream exportDeclaration;

	if (node.api.Is(Api::C))
	{
		exportDeclaration << "LOVELA_API_C ";
	}
	else if (node.api.Is(Api::Cpp))
	{
		exportDeclaration << "LOVELA_API_CPP ";
	}

	if (node.api.Is(Api::Dynamic))
	{
		exportDeclaration << "LOVELA_API_DYNAMIC_EXPORT ";
	}

	exportDeclaration << signature;

	exports.push_back(exportDeclaration.str());

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
	if (node.api.Is(Api::Standard))
	{
		// Don't emit a function declaration for standard library API:s.
		// Instead attempt to find the appropriate header to add to lovela-imports.h.

		if (node.api.Is(Api::C))
		{
			StandardCDeclarations::GetHeader(headers, node.value);
		}
		else if (node.api.Is(Api::Cpp))
		{
			StandardCppDeclarations::GetHeader(headers, node.value);
		}

		return;
	}
	else if (!node.api.IsExplicit())
	{
		// Don't emit a function declaration if the API hasn't explicitly been specified.
		// The user is expected to provide the declaration in user-imports.h.
		return;
	}

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

	if (node.api.Is(Api::C))
	{
		stream << "LOVELA_API_C ";
	}
	else if (node.api.Is(Api::Cpp))
	{
		stream << "LOVELA_API_CPP ";
	}

	if (node.api.Is(Api::Dynamic | Api::Import))
	{
		stream << "LOVELA_API_DYNAMIC_IMPORT ";
	}
	else if (node.api.Is(Api::Dynamic | Api::Export))
	{
		stream << "LOVELA_API_DYNAMIC_EXPORT ";
	}

	stream << signature << ";\n";

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

	stream << Indent() << "context;\n";;

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
	stream << LocalVar << (context.variableIndex - 1);
}

void CodeGenerator::FunctionCall(Node& node, Context& context)
{
	const auto reset = BeginAssign(context, true);

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
	if (!node.left || !node.right)
	{
		errors.emplace_back(L"Child node missing in binary operation.");
		return;
	}

	const bool reset = BeginAssign(context, true);

	Visit(*node.left, context);
	stream << ' ' << node.value << ' ';
	Visit(*node.right, context);

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
	if (!context.inner)
	{
		stream << Indent() << "const auto " << LocalVar << ++context.variableIndex << " = ";
	}
}

bool CodeGenerator::BeginAssign(Context& context, bool inner)
{
	BeginAssign(context);

	const auto reset = context.inner;
	context.inner = inner;
	return reset;
}

void CodeGenerator::EndAssign(Context& context)
{
	if (!context.inner)
	{
		stream << "; " << LocalVar << context.variableIndex << ";\n";
	}
}

void CodeGenerator::EndAssign(Context& context, bool reset)
{
	context.inner = reset;

	EndAssign(context);
}

void CodeGenerator::GenerateImportsHeaderFile(std::wostream& file)
{
	file << "#ifndef LOVELA_IMPORTS\n#define LOVELA_IMPORTS\n\n";

	for (auto& header : GetHeaders())
	{
		file << "#include <" << header << ">\n";
	}

	file << "\n#endif\n";
}


void CodeGenerator::GenerateExportsHeaderFile(std::wostream& file)
{
	file << "#ifndef LOVELA_EXPORTS\n#define LOVELA_EXPORTS\n\n";

	for (auto& declaration : GetExports())
	{
		file << declaration << ";\n";
	}

	file << "\n#endif\n";
}

void CodeGenerator::BeginProgramSourceFile(std::wostream& file)
{
	file << "#include \"lovela-program.h\"\n\n";
}

void CodeGenerator::EndProgramSourceFile(std::wostream&)
{
}
