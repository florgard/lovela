import LexerBase;

#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp"
using namespace boost::ut;

suite tokens = [] {
	"ParenRoundOpen type"_test = [] {
		expect(LexerBase::GetTokenType('(') == Token::Type::ParenRoundOpen);
	};

	"SeparatorDot type"_test = [] {
		expect(LexerBase::GetTokenType('.') == Token::Type::SeparatorDot);
	};

	"Empty type"_test = [] {
		expect(LexerBase::GetTokenType(' ') == Token::Type::Empty);
	};

	"Compare empty"_test = [] {
		expect(Token{} == Token{});
	};

	"Non-empty type"_test = [] {
		expect(Token{ Token::Type::Identifier } != Token{});
	};

	"Named empty"_test = [] {
		expect(Token{ {}, L"a" } != Token{});
	};

	"Equals"_test = [] {
		expect(Token{ Token::Type::Identifier, L"a" } == Token{ Token::Type::Identifier, L"a" });
	};

	"Unequal type"_test = [] {
		expect(Token{ Token::Type::Identifier, L"a" } != Token{ Token::Type::LiteralString, L"a" });
	};

	"Unequal name"_test = [] {
		expect(Token{ Token::Type::Identifier, L"a" } != Token{ Token::Type::Identifier, L"b" });
	};

	"Compare empty to false"_test = [] {
		expect(Token{} == false);
	};

	"Compare named empty to false"_test = [] {
		expect(Token{ {}, L"a" } == false);
	};

	"Compare non-empty to true"_test = [] {
		expect(Token{ Token::Type::Identifier } == true);
	};
};
