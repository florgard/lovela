#pragma once
#include "ParserBase.h"
#include "ILexer.h"

class Parser : public ParserBase
{
public:
	Parser(TokenGenerator&& tokenGenerator) noexcept;

	[[nodiscard]] Node Parse() noexcept override;

private:
	[[nodiscard]] TypeSpec ParseTypeSpec();
	[[nodiscard]] ParameterList ParseParameterList();
	[[nodiscard]] Node ParseFunctionDeclaration();
	[[nodiscard]] Node ParseStatement();
	[[nodiscard]] Node ParseStatements();
	[[nodiscard]] Node ParseExpression();
	[[nodiscard]] Node ParseGroup();
};
