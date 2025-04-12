#pragma once
#include "ICoder.h"
#
class CoderCpp : public ICoder
{
public:
	CoderCpp() noexcept = default;
	CoderCpp(OutputT& output) noexcept;

	void InitializeOutput(OutputT& output) noexcept
	{
		streamPtr = &output;
	}

	void Code() noexcept override;

	void Visit(Node& node) noexcept;

	[[nodiscard]] const std::vector<std::string>& GetErrors() const noexcept
	{
		return errors;
	}

	[[nodiscard]] const std::vector<std::string>& GetImports() const noexcept
	{
		return headers;
	}

	[[nodiscard]] const std::vector<std::string>& GetExports() const noexcept
	{
		return exports;
	}

	void GenerateProgramFile(std::ostream& file) const;
	void GenerateImportsFile(std::ostream& file) const;
	void GenerateExportsFile(std::ostream& file) const;

private:
	struct Context
	{
		size_t variableIndex{};
		bool inner{};
	};

	void Visit(Context& context, Node& node);

	void Visit(Context& context, std::vector<Node>& nodes)
	{
		for (auto& node : nodes)
		{
			Visit(context, node);
		}
	}

	void Visit(Context& context, std::vector<Node>& nodes, size_t start, size_t count)
	{
		for (size_t i = start, e = nodes.size(), c = 0; i < e && c < count; ++i, ++c)
		{
			Visit(context, nodes[i]);
		}
	}

	[[nodiscard]] constexpr OutputT& Cursor() noexcept
	{
		return *streamPtr;
	}

	[[nodiscard]] OutputT& Scope() noexcept
	{
		*streamPtr << '\n';
		for (size_t i = 0; i < indent; ++i)
		{
			*streamPtr << ' ' << ' ';
		}
		return *streamPtr;
	}

	OutputT& NewLine() noexcept
	{
		*streamPtr << '\n';
		return *streamPtr;
	}

	void FunctionDeclarationVisitor(Node& node, Context& context);
	void ExpressionVisitor(Node& node, Context& context);
	void ExpressionInputVisitor(Node& node, Context& context);
	void FunctionCallVisitor(Node& node, Context& context);
	void BinaryOperationVisitor(Node& node, Context& context);
	void LiteralVisitor(Node& node, Context& context);
	void TupleVisitor(Node& node, Context& context);
	void VariableReferenceVisitor(Node& node, Context& context);
	void ErrorVisitor(Node& node, Context& context);

	void MainFunctionDeclaration(Node& node, Context& context);
	void ExportedFunctionDeclaration(Node& node, Context& context);
	void ImportedFunctionDeclaration(Node& node, Context& context);
	void FunctionBody(Node& node, Context& context);
	void ImportedFunctionBody(Node& node, Context& context, const std::vector<std::pair<std::string, std::string>>& parameters);

	void BeginScope();
	void EndScope();
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
	std::optional<std::string> ConvertPrimitiveType(const TypeSpec& type);

	using Visitor = std::function<void(CoderCpp*, Node&, Context&)>;
	static std::map<Node::Type, Visitor>& GetVisitors();
	static std::map<Node::Type, Visitor>& GetInternalVisitors();

	static const TypeSpec& GetVoidType();
	static const TypeSpec& GetVoidPtrType();

	OutputT* streamPtr{};
	size_t indent{};
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

using RangeCoderCpp = BasicRangeCoder<CoderCpp>;
using VectorCoderCpp = BasicRangeRefCoder<CoderCpp, std::vector<Node>>;

inline RangeCoderCpp& operator>>(IParser::OutputT&& nodes, RangeCoderCpp& coder)
{
	coder.Initialize(std::move(nodes));
	return coder;
}

inline void operator>>(RangeCoderCpp& coder, ICoder::OutputT& output)
{
	coder.InitializeOutput(output);
	coder.Code();
}

inline VectorCoderCpp& operator>>(std::vector<Node>& nodes, VectorCoderCpp& coder)
{
	coder.Initialize(nodes);
	return coder;
}

inline void operator>>(VectorCoderCpp& coder, ICoder::OutputT& output)
{
	coder.InitializeOutput(output);
	coder.Code();
}
