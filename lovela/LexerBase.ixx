export module LexerBase;

export import ILexer;
import <string>;
import <string_view>;
import <vector>;
import <deque>;

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

	[[nodiscard]] static Token::Type GetTokenType(wchar_t lexeme) noexcept;
	[[nodiscard]] static std::wstring_view GetStringField(wchar_t code) noexcept;

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
