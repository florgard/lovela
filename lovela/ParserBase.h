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

	// Throws InvalidCurrentTokenException.
	void Assert()
	{
		throw InvalidCurrentTokenException(currentToken);
	}

	// Asserts that the current token is of the given type.
	// Throws InvalidCurrentTokenException if the token isn't of the given type.
	constexpr void Assert(Token::Type type)
	{
		if (!IsToken(type))
		{
			throw InvalidCurrentTokenException(currentToken);
		}
	}

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

	// Skips the current token and sets the next token as current token.
	constexpr void Skip()
	{
		currentToken = *GetTokenIterator();
		GetTokenIterator()++;
	}

	// Skips the current token and sets the next token as current token, if the next token is of the given type.
	constexpr void Skip(Token::Type type)
	{
		if (Peek(type))
		{
			Skip();
		}
	}

	// Skips the current token and sets the next token as current token, if the next token is of one of the given types.
	template <size_t Size>
	constexpr void Skip(const static_set<Token::Type, Size>& types) 
	{
		if (Peek(types))
		{
			Skip();
		}
	}

	// Checks whether the current token is of the given type.
	// Returns true if the token is of the given type, false otherwise.
	[[nodiscard]] constexpr bool IsToken(Token::Type type)
	{
		return currentToken.type == type;
	}

	// Checks whether the current token is of one of the given types.
	// Returns true if the token is of one of the given types, false otherwise.
	template <size_t Size>
	[[nodiscard]] constexpr bool IsToken(const static_set<Token::Type, Size>& types)
	{
		return types.contains(currentToken.type);
	}

	// Sets the current token to the next token that is expected to be of the given type.
	// Throws if the next token isn't of the given type or if there's no next token.
	constexpr void Expect(Token::Type type)
	{
		if (!Accept(type))
		{
			throw UnexpectedTokenException(*GetTokenIterator(), type);
		}
	}

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

	// Sets the current token to the next token if it's of the given type.
	// Returns true if the token was set, false otherwise.
	[[nodiscard]] constexpr bool Accept(Token::Type type)
	{
		if (Peek(type))
		{
			Skip();
			return true;
		}

		return false;
	}

	// Sets the current token to the next token if it's of one of the given types.
	// Returns true if the token was set, false otherwise.
	template <size_t Size>
	[[nodiscard]] constexpr bool Accept(const static_set<Token::Type, Size>& types)
	{
		return types.contains_if([&](Token::Type type) { return Accept(type); });
	}

	// Checks whether there's a next token.
	// Returns true if there's a token, false otherwise.
	[[nodiscard]] constexpr bool Peek()
	{
		return !GetTokenIterator().empty();
	}

	// Checks whether the next token is of the given type.
	// Returns true if the token is of the given type, false otherwise.
	[[nodiscard]] constexpr bool Peek(Token::Type type)
	{
		if (!Peek())
		{
			return false;
		}

		const auto nextType = GetTokenIterator()->type;
		return type == nextType;
	}

	// Checks whether the next token is of one of the given types.
	// Returns true if the token is of one of the given types, false otherwise.
	template <size_t Size>
	[[nodiscard]] constexpr bool Peek(const static_set<Token::Type, Size>& types)
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
