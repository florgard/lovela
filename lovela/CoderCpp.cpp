#include "pch.h"
#include "CoderCpp.h"
#include "StandardCDeclarations.h"
#include "StandardCppDeclarations.h"

std::map<Node::Type, CodeGeneratorCpp::Visitor>& CodeGeneratorCpp::GetVisitors()
{
	static std::map<Node::Type, Visitor> visitors
	{
		{ Node::Type::Error,&CodeGeneratorCpp::ErrorVisitor },
		{ Node::Type::FunctionDeclaration, &CodeGeneratorCpp::FunctionDeclarationVisitor },
	};

	return visitors;
}

std::map<Node::Type, CodeGeneratorCpp::Visitor>& CodeGeneratorCpp::GetInternalVisitors()
{
	static std::map<Node::Type, Visitor> visitors
	{
		{ Node::Type::Expression, &CodeGeneratorCpp::ExpressionVisitor },
		{ Node::Type::ExpressionInput, &CodeGeneratorCpp::ExpressionInputVisitor },
		{ Node::Type::FunctionCall, &CodeGeneratorCpp::FunctionCallVisitor },
		{ Node::Type::BinaryOperation, &CodeGeneratorCpp::BinaryOperationVisitor },
		{ Node::Type::Literal, &CodeGeneratorCpp::LiteralVisitor },
		{ Node::Type::Tuple, &CodeGeneratorCpp::TupleVisitor },
		{ Node::Type::VariableReference, &CodeGeneratorCpp::VariableReferenceVisitor },
	};

	return visitors;
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

CodeGeneratorCpp::CodeGeneratorCpp(Stream& stream) noexcept
	: streamPtr(&stream)
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
	GetStream() << Indent() << "{\n";
	indent += '\t';
}

void CodeGeneratorCpp::EndScope()
{
	if (indent.empty())
	{
		throw std::exception("Scope begin and end mismatch.");
	}

	indent.resize(indent.length() - 1);
	GetStream() << Indent() << "}\n";
}

void CodeGeneratorCpp::ErrorVisitor(Node& node, Context&)
{
	GetStream() << "/* " << to_string(node.error.code) << ": " << node.error.message << " */\n";
}

void CodeGeneratorCpp::FunctionDeclarationVisitor(Node& node, Context& context)
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

	std::vector<std::string> templateParameters;
	std::vector<std::pair<std::string, std::string>> parameters;

	const auto outType = ConvertType(node.outType);

	if (node.outType.Is(TypeSpec::Kind::Tagged))
	{
		templateParameters.push_back(outType.name);
	}

	const auto inType = ConvertType(node.inType);
	parameters.emplace_back(std::make_pair(inType.name, "in"));

	if (node.inType.Is(TypeSpec::Kind::Tagged))
	{
		templateParameters.push_back(inType.name);
	}

	size_t index = 0;
	for (auto& parameter : node.parameters)
	{
		++index;
		const auto name = ParameterName(parameter->name, index);
		const auto type = ConvertType(parameter->type);

		if (parameter->type.Is(TypeSpec::Kind::Tagged))
		{
			templateParameters.push_back(type.name);
		}

		parameters.emplace_back(std::make_pair(type.name, name));
	}

	if (!templateParameters.empty())
	{
		GetStream() << Indent() << "template <";

		index = 0;
		for (auto& param : templateParameters)
		{
			GetStream() << (index++ ? ", " : "");
			GetStream() << "typename " << param;
		}

		GetStream() << ">\n";
	}

	GetStream() << Indent() << outType.name << ' ' << FunctionName(node.value) << "(lovela::context& context";

	for (auto& parameter : parameters)
	{
		GetStream() << ", " << parameter.first << ' ' << parameter.second;
	}

	GetStream() << ')';

	if (node.api.Is(ApiSpec::Import))
	{
		ImportedFunctionBody(node, context, parameters);
	}
	else
	{
		FunctionBody(node, context);
	}

	GetStream() << '\n';

	// Generate the exported function

	if (node.api.Is(ApiSpec::Export))
	{
		ExportedFunctionDeclaration(node, context);
	}
}

void CodeGeneratorCpp::MainFunctionDeclaration(Node& node, Context& context)
{
	if (!node.outType.Is(TypeSpec::Kind::None))
	{
		errors.emplace_back("Warning: The main function out type wasn't None. The parser should set that.");
		//node.outType = { .kind = TypeSpec::Kind::None };
	}

	GetStream() << TypeNames::none << ' ' << "lovela::main(lovela::context& context, " << TypeNames::none << " in)";
	FunctionBody(node, context);
	GetStream() << '\n';
}

std::optional<TypeSpec> CodeGeneratorCpp::CheckExportType(const TypeSpec& type)
{
	switch (type.kind)
	{
	case TypeSpec::Kind::None:
		return type;

	case TypeSpec::Kind::Any:
		return GetVoidPtrType();

	default:
	{
		auto name = ConvertPrimitiveType(type);
		if (!name.has_value())
		{
			errors.emplace_back("Error: Exported functions must have primitive in, out and parameter types. Unsupported type: " + type.name);
			return {};
		}

		auto converted = type;
		converted.name = name.value();
		return converted;
	}
	}
}

std::optional<std::string> CodeGeneratorCpp::ConvertPrimitiveType(const TypeSpec& type)
{
	static const std::map<std::string, std::string> types
	{
		{"[/type/bool]", "l_i8"},
		{"[/type/i8]#", "l_cstr"},
	};

	auto it = types.find(type.GetQualifiedName());
	if (it != types.end())
	{
		return it->second;
	}

	std::ostringstream exportName;

	exportName << "l_";

	type.PrintPrimitiveName(exportName);

	for (size_t i = 0, c = type.arrayDims.size(); i < c; ++i)
	{
		exportName << '*';
	}

	return exportName.str();
}

TypeSpec CodeGeneratorCpp::ConvertType(const TypeSpec& type)
{
	auto converted = type;
	converted.name = ConvertTypeName(type);
	return converted;
}

std::string CodeGeneratorCpp::ConvertTypeName(const TypeSpec& type)
{
	switch (type.kind)
	{
	case TypeSpec::Kind::Any:
		return TypeNames::any;

	case TypeSpec::Kind::None:
		return TypeNames::none;

	case TypeSpec::Kind::Tagged:
		return "Tag" + type.name;

	case TypeSpec::Kind::Named:
		return "t_" + type.name;

	case TypeSpec::Kind::Primitive:
		return ConvertPrimitiveType(type).value_or(TypeNames::invalid);

	case TypeSpec::Kind::Invalid:
		errors.emplace_back("Error: Invalid type encountered at code generation.");
		return TypeNames::invalid;

	default:
		errors.emplace_back(fmt::format("Error: Unhandled type kind \"{}\" when getting the target type name.", to_string(type.kind)));
		return TypeNames::invalid;
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
	std::vector<std::pair<std::string, std::string>> parameters;

	// Verify and convert the input type

	const auto maybeInType = CheckExportType(node.inType);
	if (!maybeInType.has_value())
	{
		return;
	}

	const auto inType = maybeInType.value();
	if (!inType.Is(TypeSpec::Kind::None))
	{
		parameters.emplace_back(std::make_pair(inType.name, "in"));
	}

	// Verify and convert the output type

	const auto maybeOutType = CheckExportType(node.outType);
	if (!maybeOutType.has_value())
	{
		return;
	}

	auto outType = maybeOutType.value();
	if (outType.Is(TypeSpec::Kind::None))
	{
		outType = GetVoidType();
	}

	// Verify and convert the parameter types

	size_t index = 0;
	for (auto& parameter : node.parameters)
	{
		index++;
		const auto name = ParameterName(parameter->name, index);

		auto maybeType = CheckExportType(parameter->type);
		if (!maybeType.has_value())
		{
			return;
		}

		parameters.emplace_back(std::make_pair(maybeType.value().name, name));
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

	GetStream() << signature << '\n';

	BeginScope();

	GetStream() << Indent() << "lovela::context context;\n";

	if (inType.Is(TypeSpec::Kind::None))
	{
		GetStream() << Indent() << TypeNames::none << " in;\n";
	}

	// Call the actual function

	GetStream() << Indent() << (node.outType.Is(TypeSpec::Kind::None) ? "" : "return ") << FunctionName(node.value) << "(context";

	if (inType.Is(TypeSpec::Kind::None))
	{
		GetStream() << ", in";
	}

	for (auto& parameter : parameters)
	{
		GetStream() << ", " << parameter.second;
	}

	GetStream() << ");\n";

	EndScope();

	GetStream() << '\n';
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

	std::vector<std::pair<std::string, std::string>> parameters;

	// Verify and convert the input type

	const auto maybeInType = CheckExportType(node.inType);
	if (!maybeInType.has_value())
	{
		return;
	}

	const auto inType = maybeInType.value();
	if (!inType.Is(TypeSpec::Kind::None))
	{
		parameters.emplace_back(std::make_pair(inType.name, "in"));
	}

	// Verify and convert the output type

	const auto maybeOutType = CheckExportType(node.outType);
	if (!maybeOutType.has_value())
	{
		return;
	}

	auto outType = maybeOutType.value();
	if (outType.Is(TypeSpec::Kind::None))
	{
		outType = GetVoidType();
	}

	// Verify and convert the parameter types

	size_t index = 0;
	for (auto& parameter : node.parameters)
	{
		index++;
		const auto name = ParameterName(parameter->name, index);

		auto maybeType = CheckExportType(parameter->type);
		if (!maybeType.has_value())
		{
			return;
		}

		parameters.emplace_back(std::make_pair(maybeType.value().name, name));
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
		GetStream() << "LOVELA_API_C ";
	}
	else if (node.api.Is(ApiSpec::Cpp))
	{
		GetStream() << "LOVELA_API_CPP ";
	}

	if (node.api.Is(ApiSpec::Dynamic | ApiSpec::Import))
	{
		GetStream() << "LOVELA_API_DYNAMIC_IMPORT ";
	}
	else if (node.api.Is(ApiSpec::Dynamic | ApiSpec::Export))
	{
		GetStream() << "LOVELA_API_DYNAMIC_EXPORT ";
	}

	GetStream() << signature << ";\n";

	GetStream() << '\n';
}

void CodeGeneratorCpp::FunctionBody(Node& node, Context& context)
{
	if (node.left)
	{
		GetStream() << '\n';

		BeginScope();

		GetStream() << Indent() << "static_cast<void>(context);\n";

		// Make an indexed reference to the input object and avoid a warning if it's unreferenced.
		GetStream() << Indent() << "auto& " << LocalVar << ++context.variableIndex << " = in; " << RefVar(LocalVar, context.variableIndex) << ";\n";

		Visit(*node.left, context);

		if (node.outType.Is(TypeSpec::Kind::None))
		{
			GetStream() << Indent() << "return {};\n";
		}
		else
		{
			GetStream() << Indent() << "return " << LocalVar << context.variableIndex << ";\n";
		}

		EndScope();
	}
	else
	{
		GetStream() << ";\n";
	}
}

void CodeGeneratorCpp::ImportedFunctionBody(Node& node, Context&, const std::vector<std::pair<std::string, std::string>>& parameters)
{
	GetStream() << '\n';

	BeginScope();

	GetStream() << Indent() << "static_cast<void>(context);\n";;

	GetStream() << Indent();

	if (!node.outType.Is(TypeSpec::Kind::None))
	{
		GetStream() << "return ";
	}

	GetStream() << node.value << '(';

	size_t index = 0;
	for (auto& parameter : parameters)
	{
		GetStream() << (index++ ? ", " : "");
		GetStream() << parameter.second;
	}

	GetStream() << ");\n";

	if (node.outType.Is(TypeSpec::Kind::None))
	{
		GetStream() << "return {};\n";
	}

	EndScope();
}

void CodeGeneratorCpp::ExpressionVisitor(Node& node, Context& context)
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

void CodeGeneratorCpp::ExpressionInputVisitor(Node&, Context& context)
{
	// The input of an expression is the output of the previous expression.
	GetStream() << LocalVar << (context.variableIndex - 1);
}

void CodeGeneratorCpp::FunctionCallVisitor(Node& node, Context& context)
{
	const auto reset = BeginAssign(context, true);

	GetStream() << FunctionName(node.value) << "(context";

	if (node.left)
	{
		GetStream() << ", ";
		Visit(*node.left, context);
	}

	if (node.right)
	{
		GetStream() << ", ";
		Visit(*node.right, context);
	}

	GetStream() << ')';

	EndAssign(context, reset);
}

void CodeGeneratorCpp::BinaryOperationVisitor(Node& node, Context& context)
{
	if (!node.left || !node.right)
	{
		errors.emplace_back("Child node missing in binary operation.");
		return;
	}

	const bool reset = BeginAssign(context, true);

	Visit(*node.left, context);
	GetStream() << ' ' << node.value << ' ';
	Visit(*node.right, context);

	EndAssign(context, reset);
}

void CodeGeneratorCpp::LiteralVisitor(Node& node, Context& context)
{
	BeginAssign(context);
	GetStream() << (node.token.type == Token::Type::LiteralString ? double_quote(node.value) : node.value);
	EndAssign(context);
}

void CodeGeneratorCpp::TupleVisitor(Node& node, Context& context)
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
			GetStream() << ", ";
		}

		Visit(*node.right, context);
	}
}

void CodeGeneratorCpp::VariableReferenceVisitor(Node& node, Context&)
{
	GetStream() << ParameterName(node.value);
}

void CodeGeneratorCpp::BeginAssign(Context& context)
{
	if (!context.inner)
	{
		GetStream() << Indent() << "const auto " << LocalVar << ++context.variableIndex << " = ";
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
		GetStream() << "; " << RefVar(LocalVar, context.variableIndex) << ";\n";
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

	file << GetStream().rdbuf();
}