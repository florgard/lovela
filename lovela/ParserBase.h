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

	void Assert();
	void Assert(Token::Type type);
	void Assert(const std::set<Token::Type>& types);
	void Skip();
	void Skip(Token::Type type);
	void Skip(const std::set<Token::Type>& types); 
	[[nodiscard]] bool IsToken(Token::Type type);
	[[nodiscard]] bool IsToken(const std::set<Token::Type>& types);

	void Expect(Token::Type type);
	void Expect(const std::set<Token::Type>& types);
	[[nodiscard]] bool Accept(Token::Type type);
	[[nodiscard]] bool Accept(const std::set<Token::Type>& types);
	[[nodiscard]] bool Peek();
	[[nodiscard]] bool Peek(Token::Type type);
	[[nodiscard]] bool Peek(const std::set<Token::Type>& types);

protected:
	TokenGenerator tokenGenerator;
	TokenGenerator::iterator tokenIterator;
	Token currentToken;
	std::vector<Error> errors;
};
