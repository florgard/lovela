#pragma once
#include "ICodeGenerator.h"

class CodeGeneratorCpp : public ICodeGenerator
{
	friend class CodeGeneratorFactory;

protected:
	CodeGeneratorCpp(std::ostream& stream);

public:
	[[nodiscard]] const std::vector<std::string>& GetErrors() const noexcept override { return errors; }
	[[nodiscard]] const std::vector<std::string>& GetImports() const noexcept override { return headers; }
	[[nodiscard]] const std::vector<std::string>& GetExports() const noexcept override { return exports; }

	void Visit(Node& node) override;

	void GenerateProgramFile(std::ostream& file) const override;
	void GenerateImportsFile(std::ostream& file) const override;
	void GenerateExportsFile(std::ostream& file) const override;

private:
	struct Context
	{
		size_t variableIndex{};
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
	void ImportedFunctionBody(Node& node, Context& context, const std::vector<std::pair<std::string, std::string>>& parameters);

	void BeginScope();
	void EndScope();
	const std::string& Indent() const { return indent; }
	void BeginAssign(Context& context);
	bool BeginAssign(Context& context, bool set);
	void EndAssign(Context& context);
	void EndAssign(Context& context, bool reset);

	std::string ConvertTypeName(const TypeSpec& type);
	TypeSpec ConvertType(const TypeSpec& type);
	static std::string ParameterName(const std::string& name);
	static std::string ParameterName(const std::string& name, size_t index);
	static std::string FunctionName(const std::string& name);
	static std::string RefVar(char prefix, size_t index);

	std::optional<TypeSpec> CheckExportType(const TypeSpec& type);
	std::optional<std::string> ConvertPrimitiveType(const std::string& name);

	using Visitor = std::function<void(CodeGeneratorCpp*, Node&, Context&)>;
	static std::map<Node::Type, Visitor>& GetVisitors();
	static std::map<Node::Type, Visitor>& GetInternalVisitors();

	static const TypeSpec& GetVoidType();
	static const TypeSpec& GetVoidPtrType();

	std::ostream& stream;
	std::string indent;
	std::vector<std::string> errors;
	std::vector<std::string> headers;
	std::vector<std::string> exports;

	static constexpr char LocalVar{ 'v' };

	struct TypeNames
	{
		static constexpr const char* none{ "lovela::None" };
		static constexpr const char* any{ "auto" };
		static constexpr const char* invalid{ "InvalidTypeName" };
	};
};
