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

	void Assert(Token::Type type);
	void Assert(const std::vector<Token::Type>& types);
	[[nodiscard]] bool IsToken(Token::Type type);
	[[nodiscard]] bool IsToken(const std::vector<Token::Type>& types);

	void Expect(Token::Type type);
	void Expect(const std::vector<Token::Type>& types);
	[[nodiscard]] bool Accept(Token::Type type);
	[[nodiscard]] bool Accept(const std::vector<Token::Type>& types);
	[[nodiscard]] bool Peek(Token::Type type);
	[[nodiscard]] bool Peek(const std::vector<Token::Type>& types);
	void Next();

protected:
	TokenGenerator tokenGenerator;
	TokenGenerator::iterator tokenIterator;
	Token currentToken;
	std::vector<Error> errors;
};
