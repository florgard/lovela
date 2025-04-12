#pragma once
#include "ILexer.h"
#include "LexerPatterns.h"

class LexerBase : public ILexer
{
public:
	void PrintErrorSourceCode(std::ostream& stream, const Token& token) noexcept;

protected:
	[[nodiscard]] virtual Token GetToken(char lexeme) noexcept = 0;
	[[nodiscard]] virtual Token GetToken(std::string_view lexeme) noexcept = 0;

	[[nodiscard]] constexpr char GetCharacter(size_t index) const noexcept
	{
		return characters[index];
	}

	[[nodiscard]] constexpr void AddCharacter(char ch) noexcept
	{
		currentLexeme += ch;
	}

	void GetNextCharacter() noexcept;
	[[nodiscard]] Token GetCurrentToken() noexcept;
	[[nodiscard]] Token YieldToken(Token token) noexcept;
	void AddCodeLine() noexcept;

	[[nodiscard]] Token WordBreak() noexcept;
	void ExpectWordBreak() noexcept;
	[[nodiscard]] bool IsWordBreakExpected() const noexcept;

	[[nodiscard]] bool Accept() noexcept;

	[[nodiscard]] bool Accept(char pattern) noexcept;
	[[nodiscard]] bool AcceptBegin(char pattern) noexcept;

	[[nodiscard]] bool Accept(LexerPatterns::Chars pattern) noexcept;
	[[nodiscard]] bool AcceptBegin(LexerPatterns::Chars pattern) noexcept;

	/// <summary>
	/// Checks if the next 1 or 2 characters match the given regex.
	/// </summary>
	/// <typeparam name="length">The number of characters to match. Must be 1 or 2.</typeparam>
	/// <param name="pattern">The pattern with a regex and length to match.</param>
	/// <returns>true on match, false owtherwise.</returns>
	template <int length>
	bool Accept(const LexerPatterns::Regex<length>& pattern) noexcept
	{
		const auto* str = &characters[Next];
		if (std::regex_match(str, str + length, pattern.regex))
		{
			return Accept();
		}

		return false;
	}

	template <size_t length>
	[[nodiscard]] bool AcceptBegin(const LexerPatterns::Regex<length>& pattern) noexcept
	{
		return currentLexeme.empty() && Accept(pattern);
	}

	static constexpr size_t Current = 0;
	static constexpr size_t Next = 1;
	static constexpr size_t NextAfter = 2;

private:
	std::istream* _charStream{};
	std::array<char, 3> characters{};
	std::string currentLexeme;
	bool expectWordBreak{};

	size_t currentLine = 1;
	size_t currentTokenColumn = 1;
	size_t nextTokenColumn = 1;
	std::ostringstream currentSourceCode;

	size_t firstSourceCodeLine = 1;
	std::deque<std::string> sourceCodeLines;

	struct Color
	{
		std::string_view none = "\033[0m";
		std::string_view fail = "\033[97m\033[41m";
		std::string_view code = "\033[96m";
	} color;
};
