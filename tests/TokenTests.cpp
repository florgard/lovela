#include "pch.h"
#include "../lovela/LexerBase.h"

suite token_tests = [] {
	"ParenRoundOpen type"_test = [] { expect(LexerBase::GetTokenType('(') == Token::Type::ParenRoundOpen); };
	"SeparatorDot type"_test = [] { expect(LexerBase::GetTokenType('.') == Token::Type::SeparatorDot); };
	"Empty type"_test = [] { expect(LexerBase::GetTokenType(' ') == Token::Type::Empty); };
	"Compare empty"_test = [] { expect(Token{} == Token{}); };
	"Non-empty type"_test = [] { expect(Token{ Token::Type::Identifier } != Token{}); };
	"Named empty"_test = [] { expect(Token{ {}, "a" } != Token{}); };
	"Equals"_test = [] { expect(Token{ Token::Type::Identifier, "a" } == Token{ Token::Type::Identifier, "a" }); };
	"Unequal type"_test = [] { expect(Token{ Token::Type::Identifier, "a" } != Token{ Token::Type::LiteralString, "a" }); };
	"Unequal name"_test = [] { expect(Token{ Token::Type::Identifier, "a" } != Token{ Token::Type::Identifier, "b" }); };
	"Compare empty to false"_test = [] { expect(Token{} == false); };
	"Compare named empty to false"_test = [] { expect(Token{ {}, "a" } == false); };
	"Compare non-empty to true"_test = [] { expect(Token{ Token::Type::Identifier } == true); };
};
