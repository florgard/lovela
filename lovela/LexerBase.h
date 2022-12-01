#pragma once
#include "ILexer.h"
#include "LexerRegexes.h"

class LexerBase : public ILexer
{
protected:
	LexerBase(std::istream& charStream) noexcept;

	void PrintErrorSourceCode(std::ostream& stream, const Token& token) noexcept override;

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

	[[nodiscard]] constexpr std::vector<Token>& GetCurrentTokens() noexcept
	{
		return currentTokens;
	}

	[[nodiscard]] constexpr void ClearCurrentTokens() noexcept
	{
		currentTokens.clear();
	}

	void GetNextCharacter() noexcept;
	void AddCodeLine() noexcept;
	void AddToken(Token&& token) noexcept;
	void AddCurrenToken() noexcept;

	void WordBreak() noexcept;
	void ExpectWordBreak() noexcept;
	[[nodiscard]] bool IsWordBreakExpected() const noexcept;

	[[nodiscard]] bool Accept() noexcept;
	[[nodiscard]] bool Accept(char character) noexcept;
	[[nodiscard]] bool Accept(const std::regex& regex, size_t length) noexcept;
	[[nodiscard]] bool Accept(const LexerRegexes::Regex& pattern) noexcept;
	[[nodiscard]] bool AcceptBegin(char character) noexcept;
	[[nodiscard]] bool AcceptBegin(const std::regex& regex, size_t length) noexcept;
	[[nodiscard]] bool Expect(char character) noexcept;
	[[nodiscard]] bool Expect(const std::regex& regex, size_t length) noexcept;

	static constexpr size_t Current = 0;
	static constexpr size_t Next = 1;
	static constexpr size_t NextAfter = 2;

private:
	std::istream& charStream;
	std::array<char, 3> characters{};
	std::string currentLexeme;
	std::vector<Token> currentTokens;
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
