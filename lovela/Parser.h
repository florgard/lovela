#pragma once
#include "ParserBase.h"
#include "ILexer.h"

class Parser : public ParserBase
{
public:
	Parser(TokenGenerator&& tokenGenerator) noexcept;

	[[nodiscard]] Node Parse() noexcept override;

private:
	class Context
	{
	public:
		std::shared_ptr<Context> parent;
		std::set<std::wstring> symbols;
	};

	[[nodiscard]] TypeSpec ParseTypeSpec();
	[[nodiscard]] ParameterList ParseParameterList();

	[[nodiscard]] Node ParseFunctionDeclaration(std::shared_ptr<Context> context);
	[[nodiscard]] Node ParseStatement(std::shared_ptr<Context> context);
	[[nodiscard]] Node ParseStatements(std::shared_ptr<Context> context);
	[[nodiscard]] Node ParseExpression(std::shared_ptr<Context> context);
	[[nodiscard]] Node ParseGroup(std::shared_ptr<Context> context);
	[[nodiscard]] Node ParseOperand(std::shared_ptr<Context> context);
	[[nodiscard]] Node ParseFunctionCall(std::shared_ptr<Context> context);
	[[nodiscard]] Node ParseBinaryOperation(std::shared_ptr<Context> context);
};
