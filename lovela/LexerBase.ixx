export module LexerBase;

import <array>;
import <string>;
import <string_view>;
import <vector>;
import <deque>;
export import <regex>;
export import Utility.StaticMap;
export import ILexer;

export class LexerBase : public ILexer
{
public:
	[[nodiscard]] const std::vector<Error>& GetErrors() noexcept override
	{
		return errors;
	}

	[[nodiscard]] static constexpr std::wstring_view Trim(const std::wstring_view& input) noexcept
	{
		constexpr wchar_t whitespace[7]{ L" \t\r\n\f\v" };
		const auto start = input.find_first_not_of(whitespace);
		if (start == input.npos)
		{
			return {};
		}

		const auto end = input.find_last_not_of(whitespace);
		return input.substr(start, end - start + 1);
	}

	[[nodiscard]] static constexpr Token::Type GetTokenType(wchar_t lexeme) noexcept
	{
		constexpr std::array<std::pair<wchar_t, Token::Type>, 13> values
		{ {
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
			{'|', Token::Type::SeparatorVerticalLine },
			{':', Token::Type::SeparatorColon },
			{'#', Token::Type::SeparatorHash },
		} };

		constexpr auto map = StaticMap<wchar_t, Token::Type, values.size()>{ {values} };
		return map.at_or(lexeme, Token::Type::Empty);
	}

	[[nodiscard]] constexpr static std::wstring_view GetStringField(wchar_t code) noexcept
	{
		constexpr std::array<std::pair<wchar_t, std::wstring_view>, 3> values
		{ {
			{'t', L"\t"},
			{'n', L"\n"},
			{'r', L"\r"},
		} };

		constexpr auto map = StaticMap<wchar_t, std::wstring_view, values.size()>{ {values} };
		return map.at_or(code, {});
	}

protected:
	[[nodiscard]] Token GetToken(wchar_t lexeme) noexcept;
	[[nodiscard]] Token GetToken(const std::wstring_view& lexeme) noexcept;
	void AddError(Error::Code code, const std::wstring& message);

	std::vector<Error> errors;
	int currentLine = 1;
	int currentColumn = 1;
	std::deque<wchar_t> currentCode;

	static constexpr wchar_t stringTypeName[4]{ L"#8#" };
	static constexpr wchar_t integerTypeName[5]{ L"#32" };
	static constexpr wchar_t decimalTypeName[5]{ L"#.32" };
};
