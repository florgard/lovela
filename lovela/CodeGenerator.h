#pragma once
#include "Node.h"

class CodeGenerator
{
public:
	CodeGenerator(std::wostream& stream);

	void Generate(Node& node);

	const std::vector<std::wstring>& GetErrors() { return errors; }

private:
	struct Context
	{
		int variableIndex{};
	};

	void Visit(Node& node, Context& context);

	void FunctionDeclaration(Node& node, Context& context);
	void MainFunctionDeclaration(Node& node, Context& context);
	void FunctionBody(Node& node, Context& context, const std::vector<std::wstring>& initialization);
	void Expression(Node& node, Context& context);
	void ExpressionInput(Node& node, Context& context);
	void FunctionCall(Node& node, Context& context);
	void BinaryOperation(Node& node, Context& context);
	void Literal(Node& node, Context& context);
	void VariableReference(Node& node, Context& context);

	void BeginScope();
	void EndScope();
	const std::wstring& Indent() const { return indent; }

	static std::wstring TypeName(const std::wstring& name) { return L"t_" + name; }
	static std::wstring ParameterName(const std::wstring& name) { return L"p_" + name; }
	static std::wstring FunctionName(const std::wstring& name) { return L"f_" + name; }

	std::wostream& stream;
	std::wstring indent;
	std::vector<std::wstring> errors;
	using Visitor = std::function<void(CodeGenerator*, Node&, CodeGenerator::Context&)>;
	static std::map<Node::Type, Visitor> visitors;
	static std::map<Node::Type, Visitor> internalVisitors;

	static const TypeSpec NoneType;
	static constexpr char LocalVar{ 'v' };
};
