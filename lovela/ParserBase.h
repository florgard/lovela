#pragma once
#include "ILexer.h"
#include "IParser.h"
#include "ITokenIterator.h"
#include "ParseException.h"

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

	// Asserts that the current token is of one of the given types.
	// Throws InvalidCurrentTokenException if the token isn't of one of the given types.
	template <size_t Size>
	constexpr void Assert(const static_set<Token::Type, Size>& types)
	{
		if (!IsToken(types))
		{
			throw InvalidCurrentTokenException(currentToken);
		}
	}

	void Skip();
	void Skip(Token::Type type);
	void Skip(const std::set<Token::Type>& types);
	[[nodiscard]] bool IsToken(Token::Type type);
	[[nodiscard]] bool IsToken(const std::set<Token::Type>& types);

	// Checks whether the current token is of one of the given types.
	// Returns true if the token is of one of the given types, false otherwise.
	template <size_t Size>
	constexpr bool IsToken(const static_set<Token::Type, Size>& types)
	{
		return types.contains(currentToken.type);
	}

	void Expect(Token::Type type);
	void Expect(const std::set<Token::Type>& types);

	// Sets the current token to the next token that is expected to be of one of the given types.
	// Throws if the next token isn't of one of the given types or if there's no next token.
	template <size_t Size>
	constexpr void Expect(const static_set<Token::Type, Size>& types)
	{
		if (!Accept(types))
		{
			throw UnexpectedTokenException(*GetTokenIterator(), types);
		}
	}

	[[nodiscard]] bool Accept(Token::Type type);
	[[nodiscard]] bool Accept(const std::set<Token::Type>& types);

	// Sets the current token to the next token if it's of one of the given types.
	// Returns true if the token was set, false otherwise.
	template <size_t Size>
	constexpr bool Accept(const static_set<Token::Type, Size>& types)
	{
		return types.contains_if([&](Token::Type type) { return Accept(type); });
	}

	[[nodiscard]] bool Peek();
	[[nodiscard]] bool Peek(Token::Type type);
	[[nodiscard]] bool Peek(const std::set<Token::Type>& types);

	// Checks whether the next token is of one of the given types.
	// Returns true if the token is of one of the given types, false otherwise.
	template <size_t Size>
	constexpr bool Peek(const static_set<Token::Type, Size>& types)
	{
		if (!Peek())
		{
			return false;
		}

		const auto nextType = GetTokenIterator()->type;
		return types.contains(nextType);
	}

protected:
	Token currentToken;
	std::vector<Error> errors;

protected:
	ITokenIterator& GetTokenIterator() noexcept { return *tokenIterator; };
	const ITokenIterator& GetTokenIterator() const noexcept { return *tokenIterator; };

private:
	std::unique_ptr<ITokenIterator> tokenIterator;
};
