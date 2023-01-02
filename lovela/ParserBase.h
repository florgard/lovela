#pragma once
#include "IParser.h"
#include "ParseException.h"

class ParserBase : public IParser
{
	Token currentToken;

protected:
	// Prohibit creation.
	ParserBase() = default;

	[[nodiscard]] constexpr Token& GetCurrent() noexcept
	{
		return currentToken;
	}

	// Throws InvalidCurrentTokenException.
	void Assert()
	{
		throw InvalidCurrentTokenException(GetCurrent());
	}

	// Asserts that the current token is of the given type.
	// Throws InvalidCurrentTokenException if the token isn't of the given type.
	constexpr void Assert(Token::Type type)
	{
		if (!IsToken(type))
		{
			throw InvalidCurrentTokenException(GetCurrent());
		}
	}

	// Asserts that the current token is of one of the given types.
	// Throws InvalidCurrentTokenException if the token isn't of one of the given types.
	template <size_t Size>
	constexpr void Assert(const static_set<Token::Type, Size>& types)
	{
		if (!IsToken(types))
		{
			throw InvalidCurrentTokenException(GetCurrent());
		}
	}

	// Skips the current token and sets the next token as current token.
	void Skip()
	{
		currentToken = GetNext();
		Advance();
	}

	// Skips the current token and sets the next token as current token, if the next token is of the given type.
	void Skip(Token::Type type)
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
		return GetCurrent().type == type;
	}

	// Checks whether the current token is of one of the given types.
	// Returns true if the token is of one of the given types, false otherwise.
	template <size_t Size>
	[[nodiscard]] constexpr bool IsToken(const static_set<Token::Type, Size>& types)
	{
		return types.contains(GetCurrent().type);
	}

	// Sets the current token to the next token that is expected to be of the given type.
	// Throws if the next token isn't of the given type or if there's no next token.
	void Expect(Token::Type type)
	{
		if (!Accept(type))
		{
			throw UnexpectedTokenException(GetNext(), type);
		}
	}

	// Sets the current token to the next token that is expected to be of the given type and hold the given value.
	// Throws if the next token isn't of the given type, doesn't hold the given value, or if there's no next token.
	void Expect(Token::Type type, std::string_view value)
	{
		if (!Accept(type))
		{
			throw UnexpectedTokenException(GetNext(), type);
		}

		if (GetCurrent().value != value)
		{
			throw UnexpectedTokenException(GetNext(), type, value);
		}
	}

	// Sets the current token to the next token that is expected to be of one of the given types.
	// Throws if the next token isn't of one of the given types or if there's no next token.
	template <size_t Size>
	constexpr void Expect(const static_set<Token::Type, Size>& types)
	{
		if (!Accept(types))
		{
			throw UnexpectedTokenException(GetNext(), types.data);
		}
	}

	// Sets the current token to the next token if it's of the given type.
	// Returns true if the token was set, false otherwise.
	[[nodiscard]] bool Accept(Token::Type type)
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
	// Throws ErrorTokenException if the next token is an error token.
	[[nodiscard]] bool Peek()
	{
		if (IsDone())
		{
			return false;
		}

		const auto nextType = GetNext().type;
		if (nextType == Token::Type::Error)
		{
			throw ErrorTokenException(GetNext());
		}

		return true;
	}

	// Checks whether the next token is of the given type.
	// Returns true if the token is of the given type, false otherwise.
	[[nodiscard]] bool Peek(Token::Type type)
	{
		if (!Peek())
		{
			return false;
		}

		const auto nextType = GetNext().type;
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

		const auto nextType = GetNext().type;
		return types.contains(nextType);
	}
};
