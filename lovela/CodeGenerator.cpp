#include "pch.h"
#include "CodeGenerator.h"

std::map<Node::Type, std::function<void(CodeGenerator*, Node&)>> CodeGenerator::visitors
{
	{Node::Type::Function, &CodeGenerator::FunctionDeclaration}
};

std::map<Node::Type, std::function<void(CodeGenerator*, Node&)>> CodeGenerator::internalVisitors
{
	{Node::Type::Expression, &CodeGenerator::Expression}
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
	indent++;
}

void CodeGenerator::EndScope()
{
	indent--;
	stream << GetIndent() << "}\n";
}

void CodeGenerator::FunctionDeclaration(Node& node)
{
	std::vector<std::wstring> templateParameters;
	std::vector<std::wstring> parameters;
	std::wstring returnType;
	std::wstring voidType = Decorate("void");
	std::wstring objectName = Decorate("object");
	std::vector<std::wstring> initialization;

	if (node.dataType.any)
	{
		returnType = Decorate(L"return_t");
		templateParameters.push_back(returnType);
	}
	else if (!node.dataType.none)
	{
		returnType = node.dataType.name;
	}
	else
	{
		returnType = voidType;
	}

	if (node.objectType.any)
	{
		auto type = objectName + L"_t";
		parameters.emplace_back(type + L' ' + objectName);
		templateParameters.push_back(type);
	}
	else if (!node.objectType.none)
	{
		auto type = node.objectType.name;
		parameters.emplace_back(type + L' ' + objectName);
	}
	else
	{
		initialization.emplace_back(voidType + L' ' + objectName);
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
		stream << GetIndent() << "template <";

		index = 0;
		for (auto& param : templateParameters)
		{
			stream << (index++ ? ", " : "");
			stream << "typename " << param;
		}

		stream << ">\n";
	}

	stream << GetIndent() << returnType << ' ' << node.name << '(';

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
		BeginScope();
		for (auto& line : initialization)
		{
			stream << GetIndent() << line << ";\n";
		}
		Visit(node.children.front());
		EndScope();
	}
}

void CodeGenerator::Expression(Node& node)
{
	node;
	stream << GetIndent() << "return {};\n";
}
