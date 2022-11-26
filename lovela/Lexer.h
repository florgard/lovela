#pragma once
#include "LexerBase.h"
#include "LexerRegexes.h"

class Lexer : public LexerBase
{
	friend class LexerFactory;

protected:
	Lexer(std::istream& charStream) noexcept;

public:
	[[nodiscard]] TokenGenerator Lex() noexcept override;

private:
	void AddToken(Token&& token) noexcept override;

	void GetNextCharacter() noexcept;
	void AddCurrenToken() noexcept;
	void WordBreak() noexcept;
	void ExpectWordBreak() noexcept;
	[[nodiscard]] bool IsWordBreakExpected() const noexcept;

	[[nodiscard]] bool Accept() noexcept;
	[[nodiscard]] bool Accept(char character) noexcept;
	[[nodiscard]] bool Accept(const std::regex& regex, size_t length) noexcept;
	[[nodiscard]] bool AcceptBegin(char character) noexcept;
	[[nodiscard]] bool AcceptBegin(const std::regex& regex, size_t length) noexcept;
	[[nodiscard]] bool Expect(char character) noexcept;
	[[nodiscard]] bool Expect(const std::regex& regex, size_t length) noexcept;

	[[nodiscard]] void LexLiteralString() noexcept;
	[[nodiscard]] void LexLiteralNumber() noexcept;
	[[nodiscard]] void LexComment() noexcept;
	[[nodiscard]] void LexSeparator() noexcept;
	[[nodiscard]] void LexWhitespace() noexcept;

	static constexpr size_t Current = 0;
	static constexpr size_t Next = 1;
	static constexpr size_t NextAfter = 2;

	std::istream& charStream;
	std::array<char, 3> characters{};
	std::string currentLexeme;
	std::vector<Token> currentTokens;
	LexerRegexes regexes;
	bool expectWordBreak{};
};
