#pragma once
#include "ILexer.h"
#include "IParser.h"

class ParserBase : public IParser
{
public:
	ParserBase(TokenGenerator&& tokenGenerator) noexcept;

	[[nodiscard]] const std::vector<Error>& GetErrors() noexcept override
	{
		return errors;
	}

protected:
	void Expect(Token::Type type);
	void Expect(const std::vector<Token::Type>& types);
	[[nodiscard]] bool Accept(Token::Type type);
	[[nodiscard]] bool Accept(const std::vector<Token::Type>& types);
	void Next();

	TokenGenerator tokenGenerator;
	TokenGenerator::iterator tokenIterator;
	Token currentToken;
	std::vector<Error> errors;
};
