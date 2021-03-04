#pragma once
#include "ILexer.h"

class LexerBase : public ILexer
{
public:
	[[nodiscard]] const std::vector<Error>& GetErrors() noexcept override
	{
		return errors;
	}

	[[nodiscard]] static constexpr std::wstring_view Trim(const std::wstring_view& input) noexcept
	{
		const auto start = input.find_first_not_of(whitespace);
		if (start == input.npos)
		{
			return {};
		}

		const auto end = input.find_last_not_of(whitespace);
		return input.substr(start, end - start + 1);
	}

	[[nodiscard]] static Token::Type GetTokenType(wchar_t lexeme) noexcept;
	[[nodiscard]] static std::wstring_view GetStringField(wchar_t code) noexcept;

protected:
	[[nodiscard]] Token GetToken(wchar_t lexeme) noexcept;
	[[nodiscard]] Token GetToken(const std::wstring_view& lexeme) noexcept;
	void AddError(Error::Code code, const std::wstring& message);

	std::vector<Error> errors;
	int currentLine{};
	int currentColumn{};

private:
	static constexpr wchar_t whitespace[7]{ L" \t\r\n\f\v" };
};
