#include "pch.h"
#include "../lovela/LexerBase.h"

#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp"
using namespace boost::ut;

suite token_tests = [] {
	"ParenRoundOpen type"_test = [] { expect(LexerBase::GetTokenType('(') == Token::Type::ParenRoundOpen); };
	"SeparatorDot type"_test = [] { expect(LexerBase::GetTokenType('.') == Token::Type::SeparatorDot); };
	"Empty type"_test = [] { expect(LexerBase::GetTokenType(' ') == Token::Type::Empty); };
	"Compare empty"_test = [] { expect(Token{} == Token{}); };
	"Non-empty type"_test = [] { expect(Token{ .type = Token::Type::Identifier } != Token{}); };
	"Named empty"_test = [] { expect(Token{ .value = "a" } != Token{}); };
	"Equals"_test = [] { expect(Token{ .type = Token::Type::Identifier, .value = "a" } == Token{ .type = Token::Type::Identifier, .value = "a" }); };
	"Unequal type"_test = [] { expect(Token{ .type = Token::Type::Identifier, .value = "a" } != Token{ .type = Token::Type::LiteralString, .value = "a" }); };
	"Unequal name"_test = [] { expect(Token{ .type = Token::Type::Identifier, .value = "a" } != Token{ .type = Token::Type::Identifier, .value = "b" }); };
	"Compare empty to false"_test = [] { expect(Token{} == false); };
	"Compare named empty to false"_test = [] { expect(Token{ .value = "a" } == false); };
	"Compare non-empty to true"_test = [] { expect(Token{ .type = Token::Type::Identifier } == true); };
};
