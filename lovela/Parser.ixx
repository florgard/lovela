export module Parser;

import ParserBase;
import <string>;
import <map>;
import <functional>;
import <memory>;

export class Parser : public ParserBase
{
public:
	Parser(TokenGenerator&& tokenGenerator) noexcept;

	[[nodiscard]] std::unique_ptr<Node> Parse() noexcept override;

	static void TraverseDepthFirstPreorder(Node& tree, std::function<void(Node& node)> visitor) noexcept;
	static void TraverseDepthFirstPostorder(Node& tree, std::function<void(Node& node)> visitor) noexcept;

private:
	struct Context
	{
		std::shared_ptr<Context> parent;
		std::map<std::wstring, std::shared_ptr<FunctionDeclaration>> functionSymbols;
		std::map<std::wstring, std::shared_ptr<VariableDeclaration>> variableSymbols;
		TypeSpec inType{};

		[[nodiscard]] bool HasFunctionSymbol(const std::wstring& symbol) const;
		[[nodiscard]] bool HasVariableSymbol(const std::wstring& symbol) const;
		void AddFunctionSymbol(const std::wstring& symbol);
		void AddVariableSymbol(std::shared_ptr<VariableDeclaration> variable);
	};

	[[nodiscard]] TypeSpec ParseTypeSpec();
	[[nodiscard]] ParameterList ParseParameterList();

	[[nodiscard]] std::unique_ptr<Node> ParseFunctionDeclaration(std::shared_ptr<Context> context);
	[[nodiscard]] std::unique_ptr<Node> ParseCompoundExpression(std::shared_ptr<Context> context);
	[[nodiscard]] std::unique_ptr<Node> ParseExpression(std::shared_ptr<Context> context);
	[[nodiscard]] std::unique_ptr<Node> ParseGroup(std::shared_ptr<Context> context);
	[[nodiscard]] std::unique_ptr<Node> ParseTuple(std::shared_ptr<Context> context);
	[[nodiscard]] std::unique_ptr<Node> ParseOperand(std::shared_ptr<Context> context);
	[[nodiscard]] std::unique_ptr<Node> ParseFunctionCall(std::shared_ptr<Context> context);
	[[nodiscard]] std::unique_ptr<Node> ParseBinaryOperation(std::shared_ptr<Context> context);
	[[nodiscard]] std::unique_ptr<Node> ParseVariableReference(std::shared_ptr<Context> context);

	[[nodiscard]] std::unique_ptr<Node> ReduceExpression(std::unique_ptr<Node>&& expression);
};
