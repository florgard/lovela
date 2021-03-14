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
	void FunctionCall(Node& node);
	void BinaryOperation(Node& node);
	void Literal(Node& node);

	void BeginScope();
	void EndScope();
	const std::wstring& GetIndent() const { return indent; }

	template <typename T>
	static std::wstring Decorate(const T& name) { return std::wstring(L"la_") + std::wstring(std::begin(name), std::end(name)); }

	std::wostream& stream;
	std::wstring indent;
	static std::map<Node::Type, std::function<void(CodeGenerator*, Node&)>> visitors;
	static std::map<Node::Type, std::function<void(CodeGenerator*, Node&)>> internalVisitors;
};
