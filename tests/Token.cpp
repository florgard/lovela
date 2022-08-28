import LexerBase;

#define DOCTEST_CONFIG_USE_STD_HEADERS
#include "doctest.h"

TEST_CASE("test tokens") {
	SUBTEST("ParenRoundOpen type") {
		CHECK(LexerBase::GetTokenType('(') == Token::Type::ParenRoundOpen);
	}

	SUBTEST("SeparatorDot type") {
		CHECK(LexerBase::GetTokenType('.') == Token::Type::SeparatorDot);
	}

	SUBTEST("Empty type") {
		CHECK(LexerBase::GetTokenType(' ') == Token::Type::Empty);
	}

	SUBTEST("Compare empty") {
		CHECK(Token{} == Token{});
	}

	SUBTEST("Non-empty type") {
		CHECK(Token{ Token::Type::Identifier } != Token{});
	}

	SUBTEST("Named empty") {
		CHECK(Token{ {}, L"a" } != Token{});
	}

	SUBTEST("Equals") {
		CHECK(Token{ Token::Type::Identifier, L"a" } == Token{ Token::Type::Identifier, L"a" });
	}

	SUBTEST("Unequal type") {
		CHECK(Token{ Token::Type::Identifier, L"a" } != Token{ Token::Type::LiteralString, L"a" });
	}

	SUBTEST("Unequal name") {
		CHECK(Token{ Token::Type::Identifier, L"a" } != Token{ Token::Type::Identifier, L"b" });
	}

	SUBTEST("Compare empty to false") {
		CHECK(Token{} == false);
	}

	SUBTEST("Compare named empty to false") {
		CHECK(Token{ {}, L"a" } == false);
	}

	SUBTEST("Compare non-empty to true") {
		CHECK(Token{ Token::Type::Identifier } == true);
	}
}
