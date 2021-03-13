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
	void Expression(Node& node);

	void BeginScope();
	void EndScope();
	std::wstring GetIndent() const { return std::wstring(indent, L'\t'); }

	template <typename T>
	static std::wstring Decorate(const T& name) { return std::wstring(L"la_") + std::wstring(std::begin(name), std::end(name)); }

	std::wostream& stream;
	size_t indent = 0;
	static std::map<Node::Type, std::function<void(CodeGenerator*, Node&)>> visitors;
	static std::map<Node::Type, std::function<void(CodeGenerator*, Node&)>> internalVisitors;
};
