#pragma once
#include "LexerBase.h"

class Lexer : public LexerBase
{
public:
	Lexer(std::wistream& charStream) noexcept;

	[[nodiscard]] TokenGenerator Lex() noexcept override;

private:
	Token AddToken(Token token) const;

	[[nodiscard]] bool Peek(wchar_t token);
	bool Accept();
	[[nodiscard]] bool Accept(wchar_t token);
	[[nodiscard]] bool Accept(const std::vector<wchar_t>& tokens);
	void Expect(wchar_t token);
	void Expect(const std::vector<wchar_t>& tokens);

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
	wchar_t currentToken{};
	wchar_t previousToken{};
	wchar_t nextToken{};
	std::wstring lexeme;
	std::deque<wchar_t> currentCode;
};
