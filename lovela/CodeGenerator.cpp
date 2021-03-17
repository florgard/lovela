#include "pch.h"
#include "CodeGenerator.h"

std::map<Node::Type, std::function<void(CodeGenerator*, Node&)>> CodeGenerator::visitors
{
	{Node::Type::FunctionDeclaration, &CodeGenerator::FunctionDeclaration}
};

std::map<Node::Type, std::function<void(CodeGenerator*, Node&)>> CodeGenerator::internalVisitors
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
		iter->second(this, node);
	}
}

void CodeGenerator::Visit(Node& node)
{
	auto iter = internalVisitors.find(node.type);
	if (iter != internalVisitors.end())
	{
		iter->second(this, node);
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

void CodeGenerator::FunctionDeclaration(Node& node)
{
	static const TypeSpec voidType{ .name = L"None" };

	std::vector<std::wstring> templateParameters;
	std::vector<std::wstring> parameters;
	TypeSpec returnType = node.outType;
	TypeSpec inType = node.inType;
	std::vector<std::wstring> initialization;

	if (returnType.Any())
	{
		returnType.name = L"Out";
		templateParameters.push_back(returnType.name);
	}
	else if (node.outType.None())
	{
		returnType = voidType;
	}
	else
	{
		returnType.name += L"_t";
	}

	if (inType.Any())
	{
		parameters.emplace_back(L"In in");
		templateParameters.emplace_back(L"In");
	}
	else if (inType.None())
	{
		initialization.emplace_back(voidType.name);
	}
	else
	{
		parameters.emplace_back(inType.name + L"_t in");
	}

	int index = 0;
	for (auto& param : node.parameters)
	{
		index++;
		std::wostringstream paramIndex;
		paramIndex << index;
		const auto name = param.name + L"_v";
		auto type = param.type.name + L"_t";

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

	stream << GetIndent() << returnType.name << ' ' << node.value << '(';

	index = 0;
	for (auto& param : parameters)
	{
		stream << (index++ ? ", " : "");
		stream << param;
	}

	stream << ')';

	if (!node.left)
	{
		stream << ';';
	}
	else
	{
		stream << '\n';

		BeginScope();

		for (auto& line : initialization)
		{
			stream << GetIndent() << line << ";\n";
		}

		Visit(*node.left);

		// TODO: Return expression result or default in.
		// TODO: Check result and return type compability.
		stream << GetIndent() << "return in;\n";

		EndScope();
	}

	stream << '\n' << '\n';
}

void CodeGenerator::Expression(Node& node)
{
	stream << GetIndent();

	if (node.left)
	{
		Visit(*node.left);
	}

	stream << ";\n";

	if (node.right)
	{
		Visit(*node.right);
	}
}

void CodeGenerator::FunctionCall(Node& node)
{
	stream << node.value << '(';
	// TODO: Object and parameters
	stream << ") ";
}

void CodeGenerator::BinaryOperation(Node& node)
{
	Visit(*node.left);
	stream << node.value << ' ';
	Visit(*node.right);
}

void CodeGenerator::Literal(Node& node)
{
	stream << node.value << ' ';
}
