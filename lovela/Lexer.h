#pragma once
#include <array>
#include <vector>
#include <istream>
#include "Token.h"

class ILexer
{
public:
	struct Error
	{
		enum class Code
		{
			SyntaxError,
			CommentBracketMismatch,
		} code;
		std::wstring message;
	};

	virtual std::vector<Token> Lex(std::wistream& charStream) noexcept = 0;
	virtual const std::vector<Error>& GetErrors() noexcept = 0;
};

class LexerBase : public ILexer
{
protected:
	std::vector<Error> errors;

public:
	const std::vector<Error>& GetErrors() noexcept override
	{
		return errors;
	}

	static constexpr std::wstring_view Trim(const std::wstring_view& input) noexcept
	{
		constexpr wchar_t whitespace[]{ L" \t\r\n\f\v" };

		const auto start = input.find_first_not_of(whitespace);
		if (start == input.npos)
		{
			return {};
		}

		const auto end = input.find_last_not_of(whitespace);
		return input.substr(start, end - start + 1);
	}

	static constexpr TokenType GetTokenType(wchar_t lexeme) noexcept
	{
		constexpr std::array<std::pair<wchar_t, TokenType>, 13> characters{ {
			{'(', TokenType::ParenRoundOpen },
			{')', TokenType::ParenRoundClose },
			{'[', TokenType::ParenSquareOpen },
			{']', TokenType::ParenSquareClose },
			{'{', TokenType::ParenCurlyOpen },
			{'}', TokenType::ParenCurlyClose },
			{'.', TokenType::SeparatorDot },
			{',', TokenType::SeparatorComma },
			{'!', TokenType::SeparatorExclamation },
			{'?', TokenType::SeparatorQuestion },
			{':', TokenType::OperatorColon },
		} };

		auto iter = std::find_if(characters.begin(), characters.end(), [&](const auto& pair) { return pair.first == lexeme; });
		return (iter != characters.end()) ? iter->second : TokenType::Unknown;
	}

protected:
	void AddToken(const std::wstring_view& lexeme, std::vector<Token>& tokens);

	constexpr bool AddToken(wchar_t lexeme, std::vector<Token>& tokens) noexcept
	{
		auto type = GetTokenType(lexeme);
		if (type == TokenType::Unknown)
		{
			return false;
		}

		tokens.emplace_back(Token{ .type = type, .value = std::wstring(1, lexeme) });

		return true;
	}
};

class Lexer : public LexerBase
{
public:
	std::vector<Token> Lex(std::wistream& charStream) noexcept override;
};
