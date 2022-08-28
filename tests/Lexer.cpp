#include "pch.h"
#include "TestingBase.h"

static const Token endToken{ .type = Token::Type::End };
static constexpr auto ident = Token::Type::Identifier;

suite lexer_rudimental_tests = [] {
	"empty expression"_test = [] { expect(TestingBase::TestLexer("empty expression", L"", { endToken })); };
	"single character"_test = [] { expect(TestingBase::TestLexer("single character", L".", { {.type = Token::Type::SeparatorDot, .value = L"." }, endToken })); };
};
