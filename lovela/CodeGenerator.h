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

	static std::wstring TypeName(const std::wstring& name) { return L"t_" + name; }
	static std::wstring ParameterName(const std::wstring& name) { return L"p_" + name; }
	static std::wstring FunctionName(const std::wstring& name) { return L"f_" + name; }

	std::wostream& stream;
	std::wstring indent;
	static std::map<Node::Type, std::function<void(CodeGenerator*, Node&)>> visitors;
	static std::map<Node::Type, std::function<void(CodeGenerator*, Node&)>> internalVisitors;
};
