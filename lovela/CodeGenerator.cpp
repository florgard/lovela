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
	{Node::Type::VariableReference, &CodeGenerator::VariableReference},
};

CodeGenerator::CodeGenerator(std::wostream& stream) : stream(stream)
{
}

void CodeGenerator::Generate(Node& node)
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
	static const TypeSpec noneType{ .name = L"None" };

	TypeSpec inType = node.inType;
	TypeSpec outType = node.outType;
	std::vector<std::wstring> templateParameters;
	std::vector<std::wstring> parameters;
	std::vector<std::wstring> initialization;

	if (outType.Any())
	{
		outType.name = L"Out";
		templateParameters.push_back(outType.name);
	}
	else if (node.outType.None())
	{
		outType = noneType;
	}
	else
	{
		outType.name = TypeName(outType.name);
	}

	if (inType.Any())
	{
		parameters.emplace_back(L"In in");
		templateParameters.emplace_back(L"In");
	}
	else if (inType.None())
	{
		initialization.emplace_back(noneType.name + L" in");
	}
	else
	{
		parameters.emplace_back(TypeName(inType.name) + L" in");
	}

	int index = 0;
	for (auto& parameter : node.parameters)
	{
		index++;
		std::wostringstream paramIndex;
		paramIndex << index;
		const auto name = ParameterName(parameter->name);
		auto type = TypeName(parameter->type.name);

		if (parameter->type.Any())
		{
			type = L"Param" + paramIndex.str();
			templateParameters.push_back(type);
		}

		parameters.emplace_back(type + L' ' + name);
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

	stream << Indent() << outType.name << ' ' << FunctionName(node.value) << '(';

	index = 0;
	for (auto& param : parameters)
	{
		stream << (index++ ? ", " : "");
		stream << param;
	}

	stream << ')';

	if (node.left)
	{
		stream << '\n';

		BeginScope();

		for (auto& line : initialization)
		{
			stream << Indent() << line << ";\n";
		}

		// Make an indexed reference to the input object and avoid a warning if it's unreferenced.
		stream << Indent() << "auto& " << LocalVar << ++context.variableIndex << " = in; " << LocalVar << context.variableIndex << ";\n";

		Visit(*node.left, context);

		stream << Indent() << "return " << LocalVar << context.variableIndex << ";\n";

		EndScope();
	}
	else
	{
		stream << ';';
	}

	stream << "\n\n";
}

void CodeGenerator::Expression(Node& node, Context& context)
{
	if (node.left)
	{
		stream << Indent() << "const auto " << LocalVar << ++context.variableIndex << " = ";
		Visit(*node.left, context);
		stream << ";\n";
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
	stream << FunctionName(node.value) << "( ";

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

		// TODO: parameters
		Visit(*node.right, context);
	}

	stream << ") ";
}

void CodeGenerator::BinaryOperation(Node& node, Context& context)
{
	if (node.left)
	{
		Visit(*node.left, context);
	}
	else
	{
		stream << "???";
	}

	stream << node.value << ' ';

	if (node.right)
	{
		Visit(*node.right, context);
	}
	else
	{
		stream << "???";
	}
}

void CodeGenerator::Literal(Node& node, Context&)
{
	stream << node.value << ' ';
}

void CodeGenerator::VariableReference(Node& node, Context&)
{
	stream << ParameterName(node.value) << ' ';
}
