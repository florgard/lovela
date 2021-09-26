export module CodeGenerator.Cpp;

export import :StandardCDeclarations;
export import :StandardCppDeclarations;
import Node;
import <string>;
import <string_view>;
import <iostream>;
import <sstream>;
import <vector>;
import <array>;
import <set>;
import <map>;
import <functional>;
import <algorithm>;

export class CodeGenerator
{
public:
	CodeGenerator(std::wostream& stream);

	const std::vector<std::wstring>& GetErrors() const { return errors; }
	const std::vector<std::wstring>& GetHeaders() const { return headers; }
	const std::vector<std::wstring>& GetExports() const { return exports; }

	void Visit(Node& node);

	void GenerateImportsHeaderFile(std::wostream& file);
	void GenerateExportsHeaderFile(std::wostream& file);

	static void BeginProgramSourceFile(std::wostream& file);
	static void EndProgramSourceFile(std::wostream& file);

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
	void ImportedFunctionBody(Node& node, Context& context, const std::vector<std::pair<std::wstring, std::wstring>>& parameters);

	void BeginScope();
	void EndScope();
	const std::wstring& Indent() const { return indent; }
	void BeginAssign(Context& context);
	bool BeginAssign(Context& context, bool set);
	void EndAssign(Context& context);
	void EndAssign(Context& context, bool reset);

	std::wstring TypeName(const std::wstring& name);
	std::wstring TypeName(const std::wstring& name, size_t index);
	static std::wstring ParameterName(const std::wstring& name);
	static std::wstring ParameterName(const std::wstring& name, size_t index);
	static std::wstring FunctionName(const std::wstring& name);
	static std::wstring RefVar(wchar_t prefix, size_t index);

	bool CheckExportType(TypeSpec& type);
	bool ConvertPrimitiveType(std::wstring& name);

	std::wostream& stream;
	std::wstring indent;
	std::vector<std::wstring> errors;
	std::vector<std::wstring> headers;
	std::vector<std::wstring> exports;
	using Visitor = std::function<void(CodeGenerator*, Node&, CodeGenerator::Context&)>;
	static std::map<Node::Type, Visitor> visitors;
	static std::map<Node::Type, Visitor> internalVisitors;

	static const TypeSpec NoneType;
	static const TypeSpec VoidType;
	static const TypeSpec VoidPtrType;
	static constexpr char LocalVar{ 'v' };
};
