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
	std::vector<std::wstring> templateParameters;
	std::vector<std::wstring> parameters;
	std::wstring returnType;

	if (node.dataType.any)
	{
		returnType = Decorate(L"return_t");
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
		auto name = Decorate("object");
		auto type = name + L"_t";
		parameters.emplace_back(type + L' ' + name);
		templateParameters.push_back(type);
	}
	else if (!node.objectType.none)
	{
		auto name = Decorate("object");
		auto type = node.objectType.name;
		parameters.emplace_back(type + L' ' + name);
	}

	int index = 0;
	for (auto& param : node.parameters)
	{
		std::wostringstream paramName;
		paramName << "param" << (index++ + 1);
		auto name = param.name;
		auto type = param.type.name;

		if (param.type.any)
		{
			type = Decorate(paramName.str()) + L"_t";
			templateParameters.push_back(type);
		}

		parameters.emplace_back(type + L' ' + name);
	}

	if (!templateParameters.empty())
	{
		stream << "template <";

		index = 0;
		for (auto& param : templateParameters)
		{
			stream << (index++ ? ", " : "");
			stream << "typename " << param;
		}

		stream << ">\n";
	}

	stream << returnType << ' ' << node.name << '(';

	index = 0;
	for (auto& param : parameters)
	{
		stream << (index++ ? ", " : "");
		stream << param;
	}

	stream << ')';

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
