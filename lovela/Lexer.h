#pragma once
#include <array>
#include "ILexer.h"

class LexerBase : public ILexer
{
protected:
	std::vector<Error> errors;
	int currentLine{};
	int currentColumn{};

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
		return (iter != characters.end()) ? iter->second : TokenType::Empty;
	}

	static constexpr std::wstring_view GetStringField(wchar_t code) noexcept
	{
		switch (code)
		{
		case 't': return L"\t";
		case 'n': return L"\n";
		case 'r': return L"\r";
		default: return {};
		}
	}

protected:
	void AddToken(const std::wstring_view& lexeme, std::vector<Token>& tokens);

	constexpr bool AddToken(wchar_t lexeme, std::vector<Token>& tokens) noexcept
	{
		auto type = GetTokenType(lexeme);
		if (type == TokenType::Empty)
		{
			return false;
		}

		tokens.emplace_back(Token{ .type = type, .value = std::wstring(1, lexeme) });

		return true;
	}

	void AddError(Error::Code code, const std::wstring& message)
	{
		errors.emplace_back(Error{ .code = code, .line = currentLine, .column = currentColumn, .message = message });
	}
};

class Lexer : public LexerBase
{
public:
	std::vector<Token> Lex(std::wistream& charStream) noexcept override;
};
