#include "pch.h"
#include "CoderCpp.h"
#include "StandardCDeclarations.h"
#include "StandardCppDeclarations.h"

std::map<Node::Type, CoderCpp::Visitor>& CoderCpp::GetVisitors()
{
	static std::map<Node::Type, Visitor> visitors
	{
		{ Node::Type::Error,&CoderCpp::ErrorVisitor },
		{ Node::Type::FunctionDeclaration, &CoderCpp::FunctionDeclarationVisitor },
	};

	return visitors;
}

std::map<Node::Type, CoderCpp::Visitor>& CoderCpp::GetInternalVisitors()
{
	static std::map<Node::Type, Visitor> visitors
	{
		{ Node::Type::Expression, &CoderCpp::ExpressionVisitor },
		{ Node::Type::ExpressionInput, &CoderCpp::ExpressionInputVisitor },
		{ Node::Type::FunctionCall, &CoderCpp::FunctionCallVisitor },
		{ Node::Type::BinaryOperation, &CoderCpp::BinaryOperationVisitor },
		{ Node::Type::Literal, &CoderCpp::LiteralVisitor },
		{ Node::Type::Tuple, &CoderCpp::TupleVisitor },
		{ Node::Type::VariableReference, &CoderCpp::VariableReferenceVisitor },
	};

	return visitors;
}

const TypeSpec& CoderCpp::GetVoidType()
{
	static TypeSpec t{ .name = "void" };
	return t;
}

const TypeSpec& CoderCpp::GetVoidPtrType()
{
	static TypeSpec t{ .name = "void*" };
	return t;
}

CoderCpp::CoderCpp(OutputT& output) noexcept
	: streamPtr(&output)
{
}

void CoderCpp::Code() noexcept
{
	while (!IsDone())
	{
		Traverse<Node>::DepthFirstPostorder(GetNext(), [this](Node& node) { Visit(node); });
		Advance();
	}
}

void CoderCpp::Visit(Node& node) noexcept
{
	auto& v = GetVisitors();
	auto iter = v.find(node.type);
	if (iter != v.end())
	{
		Context context;
		iter->second(this, node, context);
	}
}

void CoderCpp::Visit(Context& context, Node& node)
{
	auto& v = GetInternalVisitors();
	auto iter = v.find(node.type);
	if (iter != v.end())
	{
		iter->second(this, node, context);
	}
}

void CoderCpp::BeginScope()
{
	Scope() << '{';
	indent += ' ';
	indent += ' ';
}

void CoderCpp::EndScope()
{
	if (indent.empty())
	{
		throw std::exception("Scope begin and end mismatch.");
	}

	indent.resize(indent.length() - 2);
	Scope() << '}';
}

void CoderCpp::ErrorVisitor(Node& node, Context&)
{
	Scope() << "/* " << to_string(node.error.code) << ": " << node.error.message << " */";
}

void CoderCpp::FunctionDeclarationVisitor(Node& node, Context& context)
{
	if (node.value.empty())
	{
		MainFunctionDeclaration(node, context);
		return;
	}
	else if (node.apiSpec.Is(ApiSpec::Import))
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
		Scope() << "template <";

		index = 0;
		for (auto& param : templateParameters)
		{
			GetStream() << (index++ ? ", " : "");
			GetStream() << "typename " << param;
		}

		GetStream() << '>';
	}

	Scope() << outType.name << ' ' << FunctionName(node.value) << "(lovela::context& context";

	for (auto& parameter : parameters)
	{
		GetStream() << ", " << parameter.first << ' ' << parameter.second;
	}

	GetStream() << ')';

	if (node.apiSpec.Is(ApiSpec::Import))
	{
		ImportedFunctionBody(node, context, parameters);
	}
	else
	{
		FunctionBody(node, context);
	}

	// Generate the exported function

	if (node.apiSpec.Is(ApiSpec::Export))
	{
		ExportedFunctionDeclaration(node, context);
	}
}

void CoderCpp::MainFunctionDeclaration(Node& node, Context& context)
{
	if (!node.outType.Is(TypeSpec::Kind::None))
	{
		errors.emplace_back("Warning: The main function out type wasn't None. The parser should set that.");
		//node.outType = { .kind = TypeSpec::Kind::None };
	}

	Scope() << TypeNames::none << ' ' << "lovela::main(lovela::context& context, " << TypeNames::none << " in)";
	FunctionBody(node, context);
}

std::optional<TypeSpec> CoderCpp::CheckExportType(const TypeSpec& type)
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

std::optional<std::string> CoderCpp::ConvertPrimitiveType(const TypeSpec& type)
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

TypeSpec CoderCpp::ConvertType(const TypeSpec& type)
{
	auto converted = type;
	converted.name = ConvertTypeName(type);
	return converted;
}

std::string CoderCpp::ConvertTypeName(const TypeSpec& type)
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

std::string CoderCpp::ParameterName(const std::string& name)
{
	return "p_" + name;
}

std::string CoderCpp::ParameterName(const std::string& name, size_t index)
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

std::string CoderCpp::FunctionName(const std::string& name)
{
	return "f_" + name;
}

std::string CoderCpp::RefVar(char prefix, size_t index)
{
	return std::string("static_cast<void>(") + prefix + to_string(index) + ')';
}

void CoderCpp::ExportedFunctionDeclaration(Node& node, Context&)
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

	if (node.apiSpec.Is(ApiSpec::C))
	{
		exportDeclaration << "LOVELA_API_C ";
	}
	else if (node.apiSpec.Is(ApiSpec::Cpp))
	{
		exportDeclaration << "LOVELA_API_CPP ";
	}

	if (node.apiSpec.Is(ApiSpec::Dynamic))
	{
		exportDeclaration << "LOVELA_API_DYNAMIC_EXPORT ";
	}

	exportDeclaration << signature;

	exports.push_back(exportDeclaration.str());

	// Define the exported function wrapper

	Scope() << signature;

	BeginScope();

	Scope() << "lovela::context context;";

	if (inType.Is(TypeSpec::Kind::None))
	{
		Scope() << TypeNames::none << " in;";
	}

	// Call the actual function

	Scope() << (node.outType.Is(TypeSpec::Kind::None) ? "" : "return ") << FunctionName(node.value) << "(context";

	if (inType.Is(TypeSpec::Kind::None))
	{
		GetStream() << ", in";
	}

	for (auto& parameter : parameters)
	{
		GetStream() << ", " << parameter.second;
	}

	GetStream() << ");";

	EndScope();
}

void CoderCpp::ImportedFunctionDeclaration(Node& node, Context&)
{
	if (node.apiSpec.Is(ApiSpec::Standard))
	{
		// Don't emit function declarations for standard library functions.
		// Instead attempt to find the appropriate header to add to lovela-imports.h.

		if (node.apiSpec.Is(ApiSpec::C))
		{
			StandardCDeclarations::GetHeader(headers, node.value);
		}
		else if (node.apiSpec.Is(ApiSpec::Cpp))
		{
			StandardCppDeclarations::GetHeader(headers, node.value);
		}

		return;
	}
	else if (!node.apiSpec.IsExplicit())
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

	Scope() << "";

	if (node.apiSpec.Is(ApiSpec::C))
	{
		GetStream() << "LOVELA_API_C ";
	}
	else if (node.apiSpec.Is(ApiSpec::Cpp))
	{
		GetStream() << "LOVELA_API_CPP ";
	}

	if (node.apiSpec.Is(ApiSpec::Dynamic | ApiSpec::Import))
	{
		GetStream() << "LOVELA_API_DYNAMIC_IMPORT ";
	}
	else if (node.apiSpec.Is(ApiSpec::Dynamic | ApiSpec::Export))
	{
		GetStream() << "LOVELA_API_DYNAMIC_EXPORT ";
	}

	GetStream() << signature << ";";
}

void CoderCpp::FunctionBody(Node& node, Context& context)
{
	if (!node.children.empty())
	{
		BeginScope();

		Scope() << "static_cast<void>(context);";

		// Make an indexed reference to the input object and avoid a warning if it's unreferenced.
		Scope() << "auto& " << LocalVar << ++context.variableIndex << " = in; " << RefVar(LocalVar, context.variableIndex) << ';';

		Visit(context, node.children);

		if (node.outType.Is(TypeSpec::Kind::None))
		{
			Scope() << "return {};";
		}
		else
		{
			Scope() << "return " << LocalVar << context.variableIndex << ';';
		}

		EndScope();

		Scope() << "";
	}
	else
	{
		GetStream() << ';';
	}
}

void CoderCpp::ImportedFunctionBody(Node& node, Context&, const std::vector<std::pair<std::string, std::string>>& parameters)
{
	BeginScope();

	Scope() << "static_cast<void>(context);";

	if (!node.outType.Is(TypeSpec::Kind::None))
	{
		Scope() << "return ";
	}

	GetStream() << node.value << '(';

	size_t index = 0;
	for (auto& parameter : parameters)
	{
		GetStream() << (index++ ? ", " : "");
		GetStream() << parameter.second;
	}

	GetStream() << ");";

	if (node.outType.Is(TypeSpec::Kind::None))
	{
		Scope() << "return {};";
	}

	EndScope();

	Scope() << "";
}

void CoderCpp::ExpressionVisitor(Node& node, Context& context)
{
	if (!node.children.empty())
	{
		BeginAssign(context);
		Visit(context, node.children);
		EndAssign(context);
	}

	Visit(context, node.children, 1);
}

void CoderCpp::ExpressionInputVisitor(Node&, Context& context)
{
	// The input of an expression is the output of the previous expression.
	GetStream() << LocalVar << (context.variableIndex - 1);
}

void CoderCpp::FunctionCallVisitor(Node& node, Context& context)
{
	const auto reset = BeginAssign(context, true);

	GetStream() << FunctionName(node.value) << "(context";

	if (!node.children.empty())
	{
		GetStream() << ", ";
		Visit(context, node.children);
	}

	if (node.children.size() > 1)
	{
		GetStream() << ", ";
		Visit(context, node.children, 1);
	}

	GetStream() << ')';

	EndAssign(context, reset);
}

void CoderCpp::BinaryOperationVisitor(Node& node, Context& context)
{
	if (node.children.empty())
	{
		errors.emplace_back("Child node missing in binary operation.");
		return;
	}

	const bool reset = BeginAssign(context, true);

	Visit(context, node.children);
	GetStream() << ' ' << node.value << ' ';
	Visit(context, node.children, 1);

	EndAssign(context, reset);
}

void CoderCpp::LiteralVisitor(Node& node, Context& context)
{
	BeginAssign(context);
	GetStream() << (node.token.type == Token::Type::LiteralString ? double_quote(node.value) : node.value);
	EndAssign(context);
}

void CoderCpp::TupleVisitor(Node& node, Context& context)
{
	const bool hasLeft = !node.children.empty();
	if (hasLeft)
	{
		Visit(context, node.children);
	}

	if (node.children.size() > 1)
	{
		if (hasLeft)
		{
			GetStream() << ", ";
		}

		Visit(context, node.children, 1);
	}
}

void CoderCpp::VariableReferenceVisitor(Node& node, Context&)
{
	GetStream() << ParameterName(node.value);
}

void CoderCpp::BeginAssign(Context& context)
{
	if (!context.inner)
	{
		Scope() << "const auto " << LocalVar << ++context.variableIndex << " = ";
	}
}

bool CoderCpp::BeginAssign(Context& context, bool inner)
{
	BeginAssign(context);

	const auto reset = context.inner;
	context.inner = inner;
	return reset;
}

void CoderCpp::EndAssign(Context& context)
{
	if (!context.inner)
	{
		GetStream() << "; " << RefVar(LocalVar, context.variableIndex) << ';';
	}
}

void CoderCpp::EndAssign(Context& context, bool reset)
{
	context.inner = reset;

	EndAssign(context);
}

void CoderCpp::GenerateImportsFile(std::ostream& file) const
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

void CoderCpp::GenerateExportsFile(std::ostream& file) const
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

void CoderCpp::GenerateProgramFile(std::ostream& file) const
{
	file << R"cpp(
#include "lovela-program.h"
)cpp";

	file << streamPtr->rdbuf();
}
