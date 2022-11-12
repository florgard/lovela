#include "pch.h"
#include "CodeGeneratorCpp.h"
#include "StandardCDeclarations.h"
#include "StandardCppDeclarations.h"

std::map<Node::Type, CodeGeneratorCpp::Visitor>& CodeGeneratorCpp::GetVisitors()
{
	static std::map<Node::Type, Visitor> visitors {
		{Node::Type::FunctionDeclaration, &CodeGeneratorCpp::FunctionDeclaration}
	};

	return visitors;
}

std::map<Node::Type, CodeGeneratorCpp::Visitor>& CodeGeneratorCpp::GetInternalVisitors()
{
	static std::map<Node::Type, Visitor> visitors {
		{Node::Type::Expression, &CodeGeneratorCpp::Expression},
		{Node::Type::ExpressionInput, &CodeGeneratorCpp::ExpressionInput},
		{Node::Type::FunctionCall, &CodeGeneratorCpp::FunctionCall},
		{Node::Type::BinaryOperation, &CodeGeneratorCpp::BinaryOperation},
		{Node::Type::Literal, &CodeGeneratorCpp::Literal},
		{Node::Type::Tuple, &CodeGeneratorCpp::Tuple},
		{Node::Type::VariableReference, &CodeGeneratorCpp::VariableReference},
	};

	return visitors;
}

const TypeSpec& CodeGeneratorCpp::GetNoneType()
{
	static TypeSpec t{ .name = "lovela::None" };
	return t;
}

const TypeSpec& CodeGeneratorCpp::GetVoidType()
{
	static TypeSpec t{ .name = "void" };
	return t;
}

const TypeSpec& CodeGeneratorCpp::GetVoidPtrType()
{
	static TypeSpec t{ .name = "void*" };
	return t;
}

CodeGeneratorCpp::CodeGeneratorCpp(std::ostream& stream) : stream(stream)
{
}

void CodeGeneratorCpp::Visit(Node& node)
{
	auto& v = GetVisitors();
	auto iter = v.find(node.type);
	if (iter != v.end())
	{
		Context context;
		iter->second(this, node, context);
	}
}

void CodeGeneratorCpp::Visit(Node& node, Context& context)
{
	auto& v = GetInternalVisitors();
	auto iter = v.find(node.type);
	if (iter != v.end())
	{
		iter->second(this, node, context);
	}
}

void CodeGeneratorCpp::BeginScope()
{
	stream << Indent() << "{\n";
	indent += '\t';
}

void CodeGeneratorCpp::EndScope()
{
	if (indent.empty())
	{
		throw std::exception("Scope begin and end mismatch.");
	}

	indent.resize(indent.length() - 1);
	stream << Indent() << "}\n";
}

void CodeGeneratorCpp::FunctionDeclaration(Node& node, Context& context)
{
	if (node.value.empty())
	{
		MainFunctionDeclaration(node, context);
		return;
	}
	else if (node.api.Is(ApiSpec::Import))
	{
		ImportedFunctionDeclaration(node, context);
	}

	auto inType = node.inType;
	auto outType = node.outType;
	std::vector<std::string> templateParameters;
	std::vector<std::pair<std::string, std::string>> parameters;

	if (outType.IsAny())
	{
		outType.name = "auto";
	}
	else if (node.outType.IsNone())
	{
		outType = GetNoneType();
	}
	else
	{
		outType.name = TypeName(outType.name);
	}

	if (inType.IsAny())
	{
		parameters.emplace_back(std::make_pair("In", "in"));
		templateParameters.emplace_back("In");
	}
	else if (inType.IsNone())
	{
		parameters.emplace_back(std::make_pair(GetNoneType().name, "in"));
	}
	else
	{
		parameters.emplace_back(std::make_pair(TypeName(inType.name), "in"));
	}

	size_t index = 0;
	for (auto& parameter : node.parameters)
	{
		index++;
		const auto name = ParameterName(parameter->name, index);
		const auto type = TypeName(parameter->type.name, index);

		if (parameter->type.IsAny())
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

	if (node.api.Is(ApiSpec::Import))
	{
		ImportedFunctionBody(node, context, parameters);
	}
	else
	{
		FunctionBody(node, context);
	}

	stream << '\n';

	// Generate the exported function

	if (node.api.Is(ApiSpec::Export))
	{
		ExportedFunctionDeclaration(node, context);
	}
}

void CodeGeneratorCpp::MainFunctionDeclaration(Node& node, Context& context)
{
	if (!node.outType.IsNone())
	{
		errors.emplace_back("Warning: The main function out type wasn't None. The parser should set that.");
		node.outType.SetNone();
	}

	stream << GetNoneType().name << ' ' << "lovela::main(lovela::context& context, " << GetNoneType().name << " in)";
	FunctionBody(node, context);
	stream << '\n';
}

bool CodeGeneratorCpp::CheckExportType(TypeSpec& type)
{
	if (type.IsAny())
	{
		type = GetVoidPtrType();
	}
	else if (!ConvertPrimitiveType(type.name))
	{
		errors.emplace_back("Error: Exported functions must have primitive in, out and parameter types. Unsupported type: " + type.name);
		return false;
	}

	return true;
}

bool CodeGeneratorCpp::ConvertPrimitiveType(std::string& name)
{
	static std::map<std::string, std::string> types{
		{"#8#", "l_cstr"},
	};

	if (types.contains(name))
	{
		name = types.at(name);
		return true;
	}

	static std::regex regex(R"(#(\.|\+)?(1|8|16|32|64)(#*))");
	std::smatch match;
	if (!std::regex_match(name, match, regex))
	{
		errors.emplace_back("Unsupported primitive type: " + name);
		return false;
	}

	const bool floatingPoint = match[1].compare(".") == 0;
	const bool unsignedInteger = match[1].compare("+") == 0;
	const auto width = match[2];
	if (floatingPoint && !(width == "32" || width == "64"))
	{
		errors.emplace_back("Unsupported primitive floating point type: " + name);
		return false;
	}

	std::string exportName = "l_";
	exportName += floatingPoint ? 'f' : (unsignedInteger ? 'u' : 'i');
	exportName += width;
	exportName += std::string(match[3].length(), '*');
	name = exportName;

	return true;
}

std::string CodeGeneratorCpp::TypeName(const std::string& name)
{
	if (name.front() == '#')
	{
		std::string converted = name;
		if (ConvertPrimitiveType(converted))
		{
			return converted;
		}
	}

	return "t_" + name;
}

std::string CodeGeneratorCpp::TypeName(const std::string& name, size_t index)
{
	if (name.empty())
	{
		return "Param" + to_string(index);
	}
	else
	{
		return TypeName(name);
	}
}

std::string CodeGeneratorCpp::ParameterName(const std::string& name)
{
	return "p_" + name;
}

std::string CodeGeneratorCpp::ParameterName(const std::string& name, size_t index)
{
	if (name.empty())
	{
		return "param" + to_string(index);
	}
	else
	{
		return ParameterName(name);
	}
}

std::string CodeGeneratorCpp::FunctionName(const std::string& name)
{
	return "f_" + name;
}

std::string CodeGeneratorCpp::RefVar(char prefix, size_t index)
{
	return std::string("static_cast<void>(") + prefix + to_string(index) + ')';
}

void CodeGeneratorCpp::ExportedFunctionDeclaration(Node& node, Context&)
{
	auto inType = node.inType;
	auto outType = node.outType;

	std::vector<std::pair<std::string, std::string>> parameters;

	// Verify and convert the input type

	if (inType.IsNone())
	{
	}
	else if (CheckExportType(inType))
	{
		parameters.emplace_back(std::make_pair(inType.name, "in"));
	}
	else
	{
		return;
	}

	// Verify and convert the output type

	if (outType.IsNone())
	{
		outType = GetVoidType();
	}
	else if (!CheckExportType(outType))
	{
		return;
	}

	// Verify and convert the parameter types

	size_t index = 0;
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

	std::stringstream ss;
	ss << outType.name << ' ' << node.value << '(';

	index = 0;
	for (auto& parameter : parameters)
	{
		ss << (index++ ? ", " : "") << parameter.first << ' ' << parameter.second;
	}

	ss << ')';

	auto signature = ss.str();

	// Store export declaration

	std::ostringstream exportDeclaration;

	if (node.api.Is(ApiSpec::C))
	{
		exportDeclaration << "LOVELA_API_C ";
	}
	else if (node.api.Is(ApiSpec::Cpp))
	{
		exportDeclaration << "LOVELA_API_CPP ";
	}

	if (node.api.Is(ApiSpec::Dynamic))
	{
		exportDeclaration << "LOVELA_API_DYNAMIC_EXPORT ";
	}

	exportDeclaration << signature;

	exports.push_back(exportDeclaration.str());

	// Define the exported function wrapper

	stream << signature << '\n';

	BeginScope();

	stream << Indent() << "lovela::context context;\n";

	if (inType.IsNone())
	{
		stream << Indent() << GetNoneType().name << " in;\n";
	}

	// Call the actual function

	stream << Indent() << (node.outType.IsNone() ? "" : "return ") << FunctionName(node.value) << "(context";

	if (inType.IsNone())
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

void CodeGeneratorCpp::ImportedFunctionDeclaration(Node& node, Context&)
{
	if (node.api.Is(ApiSpec::Standard))
	{
		// Don't emit function declarations for standard library functions.
		// Instead attempt to find the appropriate header to add to lovela-imports.h.

		if (node.api.Is(ApiSpec::C))
		{
			StandardCDeclarations::GetHeader(headers, node.value);
		}
		else if (node.api.Is(ApiSpec::Cpp))
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

	std::vector<std::pair<std::string, std::string>> parameters;

	// Verify and convert the input type

	if (inType.IsNone())
	{
	}
	else if (CheckExportType(inType))
	{
		parameters.emplace_back(std::make_pair(inType.name, "in"));
	}
	else
	{
		return;
	}

	// Verify and convert the output type

	if (outType.IsNone())
	{
		outType = GetVoidType();
	}
	else if (!CheckExportType(outType))
	{
		return;
	}

	// Verify and convert the parameter types

	size_t index = 0;
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

	std::stringstream ss;
	ss << outType.name << ' ' << node.value << '(';

	index = 0;
	for (auto& parameter : parameters)
	{
		ss << (index++ ? ", " : "") << parameter.first << ' ' << parameter.second;
	}

	ss << ')';

	auto signature = ss.str();

	// Declare import

	if (node.api.Is(ApiSpec::C))
	{
		stream << "LOVELA_API_C ";
	}
	else if (node.api.Is(ApiSpec::Cpp))
	{
		stream << "LOVELA_API_CPP ";
	}

	if (node.api.Is(ApiSpec::Dynamic | ApiSpec::Import))
	{
		stream << "LOVELA_API_DYNAMIC_IMPORT ";
	}
	else if (node.api.Is(ApiSpec::Dynamic | ApiSpec::Export))
	{
		stream << "LOVELA_API_DYNAMIC_EXPORT ";
	}

	stream << signature << ";\n";

	stream << '\n';
}

void CodeGeneratorCpp::FunctionBody(Node& node, Context& context)
{
	if (node.left)
	{
		stream << '\n';

		BeginScope();

		stream << Indent() << "static_cast<void>(context);\n";

		// Make an indexed reference to the input object and avoid a warning if it's unreferenced.
		stream << Indent() << "auto& " << LocalVar << ++context.variableIndex << " = in; " << RefVar(LocalVar, context.variableIndex) << ";\n";

		Visit(*node.left, context);

		if (node.outType.IsNone())
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

void CodeGeneratorCpp::ImportedFunctionBody(Node& node, Context&, const std::vector<std::pair<std::string, std::string>>& parameters)
{
	stream << '\n';

	BeginScope();

	stream << Indent() << "static_cast<void>(context);\n";;

	stream << Indent();

	if (!node.outType.IsNone())
	{
		stream << "return ";
	}

	stream << node.value << '(';

	size_t index = 0;
	for (auto& parameter : parameters)
	{
		stream << (index++ ? ", " : "");
		stream << parameter.second;
	}

	stream << ");\n";

	if (node.outType.IsNone())
	{
		stream << "return {};\n";
	}

	EndScope();
}

void CodeGeneratorCpp::Expression(Node& node, Context& context)
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

void CodeGeneratorCpp::ExpressionInput(Node&, Context& context)
{
	// The input of an expression is the output of the previous expression.
	stream << LocalVar << (context.variableIndex - 1);
}

void CodeGeneratorCpp::FunctionCall(Node& node, Context& context)
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

void CodeGeneratorCpp::BinaryOperation(Node& node, Context& context)
{
	if (!node.left || !node.right)
	{
		errors.emplace_back("Child node missing in binary operation.");
		return;
	}

	const bool reset = BeginAssign(context, true);

	Visit(*node.left, context);
	stream << ' ' << node.value << ' ';
	Visit(*node.right, context);

	EndAssign(context, reset);
}

void CodeGeneratorCpp::Literal(Node& node, Context& context)
{
	BeginAssign(context);
	stream << (node.token.type == Token::Type::LiteralString ? double_quote(node.value) : node.value);
	EndAssign(context);
}

void CodeGeneratorCpp::Tuple(Node& node, Context& context)
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

void CodeGeneratorCpp::VariableReference(Node& node, Context&)
{
	stream << ParameterName(node.value);
}

void CodeGeneratorCpp::BeginAssign(Context& context)
{
	if (!context.inner)
	{
		stream << Indent() << "const auto " << LocalVar << ++context.variableIndex << " = ";
	}
}

bool CodeGeneratorCpp::BeginAssign(Context& context, bool inner)
{
	BeginAssign(context);

	const auto reset = context.inner;
	context.inner = inner;
	return reset;
}

void CodeGeneratorCpp::EndAssign(Context& context)
{
	if (!context.inner)
	{
		stream << "; " << RefVar(LocalVar, context.variableIndex) << ";\n";
	}
}

void CodeGeneratorCpp::EndAssign(Context& context, bool reset)
{
	context.inner = reset;

	EndAssign(context);
}

void CodeGeneratorCpp::GenerateImportsFile(std::ostream& file) const
{
	file << R"cpp(
// Automatically generated header with function declarations that the lovela program imports.
#ifndef LOVELA_IMPORTS
#define LOVELA_IMPORTS
)cpp";

	for (auto& header : GetImports())
	{
		file << "#include <" << header << ">\n";
	}

	file << R"cpp(
#endif
)cpp";
}

void CodeGeneratorCpp::GenerateExportsFile(std::ostream& file) const
{
	file << R"cpp(
// Automatically generated header with function declarations that the lovela program exports.
#ifndef LOVELA_EXPORTS
#define LOVELA_EXPORTS
)cpp";

	for (auto& declaration : GetExports())
	{
		file << declaration << ";\n";
	}

	file << R"cpp(
#endif
)cpp";
}

void CodeGeneratorCpp::GenerateProgramFile(std::ostream& file) const
{
	file << R"cpp(
#include "lovela-program.h"
)cpp";

	file << stream.rdbuf();
}
