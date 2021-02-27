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

	static constexpr Token::Type GetTokenType(wchar_t lexeme) noexcept
	{
		constexpr std::array<std::pair<wchar_t, Token::Type>, 13> characters{ {
			{'(', Token::Type::ParenRoundOpen },
			{')', Token::Type::ParenRoundClose },
			{'[', Token::Type::ParenSquareOpen },
			{']', Token::Type::ParenSquareClose },
			{'{', Token::Type::ParenCurlyOpen },
			{'}', Token::Type::ParenCurlyClose },
			{'.', Token::Type::SeparatorDot },
			{',', Token::Type::SeparatorComma },
			{'!', Token::Type::SeparatorExclamation },
			{'?', Token::Type::SeparatorQuestion },
			{':', Token::Type::OperatorColon },
		} };

		auto iter = std::find_if(characters.begin(), characters.end(), [&](const auto& pair) { return pair.first == lexeme; });
		return (iter != characters.end()) ? iter->second : Token::Type::Empty;
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
	Token GetToken(wchar_t lexeme) noexcept;
	Token GetToken(const std::wstring_view& lexeme) noexcept;

	void AddError(Error::Code code, const std::wstring& message)
	{
		errors.emplace_back(Error{ .code = code, .line = currentLine, .column = currentColumn, .message = message });
	}
};

class Lexer : public LexerBase
{
	std::wistream& charStream;

public:
	Lexer(std::wistream& charStream) noexcept;
	TokenGenerator Lex() noexcept override;
};
