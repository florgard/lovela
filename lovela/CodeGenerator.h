#pragma once
#include "Node.h"

class CodeGenerator
{
public:
	CodeGenerator(std::wostream& stream);

	void Generate(Node& node);

	const std::vector<std::wstring>& GetErrors() { return errors; }
	const std::vector<std::wstring>& GetExports() { return exports; }

private:
	struct Context
	{
		int variableIndex{};
		bool assignVariable{};
	};

	void Visit(Node& node, Context& context);

	void FunctionDeclaration(Node& node, Context& context);
	void Expression(Node& node, Context& context);
	void ExpressionInput(Node& node, Context& context);
	void FunctionCall(Node& node, Context& context);
	void BinaryOperation(Node& node, Context& context);
	void Literal(Node& node, Context& context);
	void Tuple(Node& node, Context& context);
	void VariableReference(Node& node, Context& context);

	void MainFunctionDeclaration(Node& node, Context& context);
	void ExportedFunctionDeclaration(Node& node, Context& context);
	void FunctionBody(Node& node, Context& context);

	void BeginScope();
	void EndScope();
	const std::wstring& Indent() const { return indent; }
	void BeginAssign(Context& context);
	void EndAssign(Context& context);

	static std::wstring TypeName(const std::wstring& name) { return L"t_" + name; }
	static std::wstring ParameterName(const std::wstring& name) { return L"p_" + name; }
	static std::wstring FunctionName(const std::wstring& name) { return L"f_" + name; }

	bool CheckExportType(TypeSpec& type);
	static bool ConvertExportType(std::wstring& name);

	std::wostream& stream;
	std::wstring indent;
	std::vector<std::wstring> errors;
	std::vector<std::wstring> exports;
	using Visitor = std::function<void(CodeGenerator*, Node&, CodeGenerator::Context&)>;
	static std::map<Node::Type, Visitor> visitors;
	static std::map<Node::Type, Visitor> internalVisitors;

	static const TypeSpec NoneType;
	static const TypeSpec VoidType;
	static const TypeSpec VoidPtrType;
	static constexpr char LocalVar{ 'v' };
};
