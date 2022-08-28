#pragma once
#include "ILexer.h"
#include "IParser.h"
#include "ITokenIterator.h"

class ParserBase : public IParser
{
public:
	ParserBase(std::unique_ptr<ITokenIterator> tokenIterator) noexcept;

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
	Token currentToken;
	std::vector<Error> errors;

protected:
	ITokenIterator& GetTokenIterator() noexcept { return *tokenIterator; };
	const ITokenIterator& GetTokenIterator() const noexcept { return *tokenIterator; };

private:
	std::unique_ptr<ITokenIterator> tokenIterator;
};
