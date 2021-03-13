#pragma once
#include "ParserBase.h"
#include "ILexer.h"

class Parser : public ParserBase
{
public:
	Parser(TokenGenerator&& tokenGenerator) noexcept;

	[[nodiscard]] Node Parse() noexcept override;

	static void TraverseDepthFirstPreorder(Node& tree, std::function<void(Node& node)> visitor) noexcept;
	static void TraverseDepthFirstPostorder(Node& tree, std::function<void(Node& node)> visitor) noexcept;

private:
	struct Context
	{
		std::shared_ptr<Context> parent;
		std::set<std::wstring> symbols;
	};

	[[nodiscard]] TypeSpec ParseTypeSpec();
	[[nodiscard]] ParameterList ParseParameterList();

	[[nodiscard]] Node ParseFunctionDeclaration(std::shared_ptr<Context> context);
	[[nodiscard]] Node ParseCompoundExpression(std::shared_ptr<Context> context);
	[[nodiscard]] Node ParseExpression(std::shared_ptr<Context> context);
	[[nodiscard]] Node ParseGroup(std::shared_ptr<Context> context);
	[[nodiscard]] Node ParseOperand(std::shared_ptr<Context> context);
	[[nodiscard]] Node ParseFunctionCall(std::shared_ptr<Context> context);
	[[nodiscard]] Node ParseBinaryOperation(std::shared_ptr<Context> context);
};
