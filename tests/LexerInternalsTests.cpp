#include "pch.h"

class LexerTest
{
public:
	// Internal function forwarding

	[[nodiscard]] static constexpr Token::Type GetTokenType(char lexeme) noexcept
	{
		return Lexer::GetTokenType(lexeme);
	}
};

using namespace boost::ut;

suite lexer_GetTokenType_tests = [] {
	"ParenRoundOpen type"_test = [] { expect(LexerTest::GetTokenType('(') == Token::Type::ParenRoundOpen); };
	"SeparatorDot type"_test = [] { expect(LexerTest::GetTokenType('.') == Token::Type::SeparatorDot); };
	"Empty type"_test = [] { expect(LexerTest::GetTokenType(' ') == Token::Type::Empty); };
};
