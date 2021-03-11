#include "pch.h"
#include "CodeGenerator.h"

std::map<Node::Type, std::function<void(CodeGenerator*, Node&)>> CodeGenerator::visitors
{
	{Node::Type::Function, &CodeGenerator::FunctionDeclaration}
};

std::map<Node::Type, std::function<void(CodeGenerator*, Node&)>> CodeGenerator::internalVisitors
{
	{Node::Type::Statement, &CodeGenerator::Statement}
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

void CodeGenerator::FunctionDeclaration(Node& node)
{
	std::wstring returnType;
	std::wstring objectType;
	std::vector<std::wstring> templateParameters;

	if (node.dataType.any)
	{
		returnType = Decorate(L"DataType");
		templateParameters.push_back(returnType);
	}
	else if (node.dataType.none)
	{
		returnType = L"void";
	}
	else
	{
		returnType = node.dataType.name;
	}

	if (node.objectType.any)
	{
		objectType = Decorate(L"ObjectType");
		templateParameters.push_back(objectType);
	}
	else if (node.objectType.none)
	{
	}
	else
	{
		objectType = node.objectType.name;
	}

	if (!templateParameters.empty())
	{
		stream << "template <";

		int index = 0;
		for (auto& param : templateParameters)
		{
			stream << (index++ ? ", " : "");
			stream << "typename " << param;
		}

		stream << ">\n";
	}

	stream << returnType << ' ' << node.name << '(' << (!objectType.empty() ? objectType + L' ' + Decorate(L"object") : L"") << ')';

	if (node.children.empty())
	{
		stream << ";\n";
	}
	else
	{
		stream << '\n';
		Visit(node.children.front());
	}
}

void CodeGenerator::Statement(Node& node)
{
	node;
	stream << "{\n\treturn {};\n}\n";
}
