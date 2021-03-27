#pragma once
#include "Node.h"

class CodeGenerator
{
public:
	CodeGenerator(std::wostream& stream);

	const std::vector<std::wstring>& GetErrors() { return errors; }
	const std::vector<std::wstring>& GetExports() { return exports; }

	void Visit(Node& node);

	void GenerateLibraryHeaderFile(std::wostream& file);

	static void BeginProgramSourceFile(std::wostream& file);
	static void EndProgramSourceFile(std::wostream& file);
	static void BeginLibraryHeaderFile(std::wostream& file);
	static void EndLibraryHeaderFile(std::wostream& file);

private:
	struct Context
	{
		int variableIndex{};
		bool inner{};
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
	void ImportedFunctionDeclaration(Node& node, Context& context);
	void FunctionBody(Node& node, Context& context);
	void ImportedFunctionBody(Node& node, Context& context, const std::vector<std::pair<std::wstring, std::wstring>>& parameters);

	void BeginScope();
	void EndScope();
	const std::wstring& Indent() const { return indent; }
	void BeginAssign(Context& context);
	bool BeginAssign(Context& context, bool set);
	void EndAssign(Context& context);
	void EndAssign(Context& context, bool reset);

	static std::wstring TypeName(const std::wstring& name);
	static std::wstring TypeName(const std::wstring& name, int index);
	static std::wstring ParameterName(const std::wstring& name);
	static std::wstring ParameterName(const std::wstring& name, int index);
	static std::wstring FunctionName(const std::wstring& name);

	bool CheckExportType(TypeSpec& type);
	static bool ConvertPrimitiveType(std::wstring& name);

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
