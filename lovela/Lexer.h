#pragma once
#include "LexerBase.h"

class Lexer : public LexerBase
{
public:
	Lexer(std::wistream& charStream) noexcept;

	[[nodiscard]] TokenGenerator Lex() noexcept override;

private:
	[[nodiscard]] Token GetCurrentLexemeToken();
	[[nodiscard]] Token GetCurrentCharToken();
	[[nodiscard]] Token DecorateToken(Token token) const;

	[[nodiscard]] bool Peek(wchar_t token);
	bool Accept();
	[[nodiscard]] bool Accept(wchar_t token);
	[[nodiscard]] bool Accept(const std::vector<wchar_t>& tokens);
	[[nodiscard]] bool Accept(const std::wregex& regex);
	[[nodiscard]] bool Accept(std::function<bool()> predicate);
	void Expect(wchar_t token);
	void Expect(const std::vector<wchar_t>& tokens);

	[[nodiscard]] void LexStringFieldCode(std::vector<Token>& tokens) noexcept;
	[[nodiscard]] void LexLiteralString(std::vector<Token>& tokens) noexcept;
	[[nodiscard]] void LexLiteralInteger(std::vector<Token>& tokens) noexcept;
	[[nodiscard]] void LexParenAngleOpen(std::vector<Token>& tokens) noexcept;
	[[nodiscard]] void LexParenAngleClose(std::vector<Token>& tokens) noexcept;
	[[nodiscard]] void LexLiteralStringBegin(std::vector<Token>& tokens) noexcept;
	[[nodiscard]] void LexSeparator(std::vector<Token>& tokens) noexcept;
	[[nodiscard]] void LexWhitespace(std::vector<Token>& tokens) noexcept;
	[[nodiscard]] void LexLiteralIntegerBegin(std::vector<Token>& tokens) noexcept;
	[[nodiscard]] void LexPrimitiveType(std::vector<Token>& tokens) noexcept;

	struct State
	{
		bool integerLiteral = false;
		bool stringLiteral = false;
		wchar_t stringFieldCode = 0;
		wchar_t nextStringInterpolation = '1';
		int commentLevel = 0;

		void Clear()
		{
			*this = State{};
		}
	} state;

	std::wistream& charStream;
	wchar_t currentChar{};
	wchar_t previousChar{};
	wchar_t nextChar{};
	std::wstring currentLexeme;
	std::deque<wchar_t> currentCode;
};
