#include "pch.h"
#include "CodeGenerator.h"

std::map<Node::Type, CodeGenerator::Visitor> CodeGenerator::visitors
{
	{Node::Type::FunctionDeclaration, &CodeGenerator::FunctionDeclaration}
};

std::map<Node::Type, CodeGenerator::Visitor> CodeGenerator::internalVisitors
{
	{Node::Type::Expression, &CodeGenerator::Expression},
	{Node::Type::FunctionCall, &CodeGenerator::FunctionCall},
	{Node::Type::BinaryOperation, &CodeGenerator::BinaryOperation},
	{Node::Type::Literal, &CodeGenerator::Literal},
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
	stream << GetIndent() << "{\n";
	indent += '\t';
}

void CodeGenerator::EndScope()
{
	if (indent.empty())
	{
		throw std::exception("Scope begin and end mismatch.");
	}

	indent.resize(indent.length() - 1);
	stream << GetIndent() << "}\n";
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
		initialization.emplace_back(noneType.name + L" in; in;");
	}
	else
	{
		parameters.emplace_back(TypeName(inType.name) + L" in");
	}

	int index = 0;
	for (auto& param : node.parameters)
	{
		index++;
		std::wostringstream paramIndex;
		paramIndex << index;
		const auto name = ParameterName(param.name);
		auto type = TypeName(param.type.name);

		if (param.type.Any())
		{
			type = L"Param" + paramIndex.str();
			templateParameters.push_back(type);
		}

		parameters.emplace_back(type + L' ' + name);
	}

	if (!templateParameters.empty())
	{
		stream << GetIndent() << "template <";

		index = 0;
		for (auto& param : templateParameters)
		{
			stream << (index++ ? ", " : "");
			stream << "typename " << param;
		}

		stream << ">\n";
	}

	stream << GetIndent() << outType.name << ' ' << FunctionName(node.value) << '(';

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
			stream << GetIndent() << line << '\n';
		}

		Visit(*node.left, context);

		stream << GetIndent() << "return out" << context.localVariableIndex << ";\n";

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
		stream << GetIndent() << "auto out" << ++context.localVariableIndex << " = ";
		Visit(*node.left, context);
		stream << ";\n";
	}

	if (node.right)
	{
		stream << GetIndent() << "auto out" << ++context.localVariableIndex << " = ";
		Visit(*node.right, context);
		stream << ";\n";
	}
}

void CodeGenerator::FunctionCall(Node& node, Context&)
{
	stream << node.value << '(';
	// TODO: Object and parameters
	stream << ") ";
}

void CodeGenerator::BinaryOperation(Node& node, Context& context)
{
	Visit(*node.left, context);
	stream << node.value << ' ';
	Visit(*node.right, context);
}

void CodeGenerator::Literal(Node& node, Context&)
{
	stream << node.value << ' ';
}
