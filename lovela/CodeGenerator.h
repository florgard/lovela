#pragma once
#include "Node.h"

class CodeGenerator
{
public:
	CodeGenerator(std::wostream& stream);

	void Generate(Node& node);

private:
	void Visit(Node& node);

	void FunctionDeclaration(Node& node);
	void Statement(Node& node);

	std::wostream& stream;
	static std::wstring Decorate(std::wstring_view name) { return std::wstring(L"la_") + std::wstring(name.data(), name.size()); }
	static std::map<Node::Type, std::function<void(CodeGenerator*, Node&)>> visitors;
	static std::map<Node::Type, std::function<void(CodeGenerator*, Node&)>> internalVisitors;
};
