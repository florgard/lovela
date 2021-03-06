#pragma once
#include "LexerBase.h"

class Lexer : public LexerBase
{
public:
	Lexer(std::wistream& charStream) noexcept;

	[[nodiscard]] TokenGenerator Lex() noexcept override;

private:
	[[nodiscard]] Token GetCurrenToken();
	[[nodiscard]] Token DecorateToken(Token token) const;

	void GetNextCharacter() noexcept;
	[[nodiscard]] bool Accept() noexcept;
	[[nodiscard]] bool Accept(wchar_t character) noexcept;
	[[nodiscard]] bool Accept(const std::wregex& regex, size_t length) noexcept;
	[[nodiscard]] bool AcceptBegin(wchar_t character) noexcept;
	[[nodiscard]] bool AcceptBegin(const std::wregex& regex, size_t length) noexcept;
	[[nodiscard]] bool Expect(wchar_t character) noexcept;
	[[nodiscard]] bool Expect(const std::wregex& regex, size_t length) noexcept;

	[[nodiscard]] void LexLiteralString(std::vector<Token>& tokens) noexcept;
	[[nodiscard]] void LexLiteralNumber(std::vector<Token>& tokens) noexcept;
	[[nodiscard]] void LexComment(std::vector<Token>& tokens) noexcept;
	[[nodiscard]] void LexSeparator(std::vector<Token>& tokens) noexcept;
	[[nodiscard]] void LexWhitespace(std::vector<Token>& tokens) noexcept;
	[[nodiscard]] void LexPrimitiveType(std::vector<Token>& tokens) noexcept;

	static constexpr size_t Current = 0;
	static constexpr size_t Next = 1;
	static constexpr size_t NextAfter = 2;

	std::wistream& charStream;
	std::array<wchar_t, 3> characters{};
	std::wstring currentLexeme;
};
