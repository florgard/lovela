#include <iostream>
#include <sstream>
#include <cassert>
#include "Lexer.h"

void TestLexer(ILexer& lexer)
{
    {
        std::istringstream iss{ "func: 123 ." };
        const auto expected = std::vector<Token>{
            {.type{TokenType::Identifier}, .value{"func"} },
            {.type{TokenType::OperatorColon}, .value{":"} },
            {.type{TokenType::LiteralInteger}, .value{"123"} },
            {.type{TokenType::SeparatorDot}, .value{"."} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::istringstream iss{ "func : 123.4 ." };
        const auto expected = std::vector<Token>{
            {.type{TokenType::Identifier}, .value{"func"} },
            {.type{TokenType::OperatorColon}, .value{":"} },
            {.type{TokenType::LiteralDecimal}, .value{"123.4"} },
            {.type{TokenType::SeparatorDot}, .value{"."} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::istringstream iss{ "\r\nfunc44: (123.4)." };
        const auto expected = std::vector<Token>{
            {.type{TokenType::Identifier}, .value{"func44"} },
            {.type{TokenType::OperatorColon}, .value{":"} },
            {.type{TokenType::ParenRoundOpen}, .value{"("} },
            {.type{TokenType::LiteralDecimal}, .value{"123.4"} },
            {.type{TokenType::ParenRoundClose}, .value{")"} },
            {.type{TokenType::SeparatorDot}, .value{"."} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::istringstream iss{ "ident123." };
        const auto expected = std::vector<Token>{
            {.type{TokenType::Identifier}, .value{"ident123"} },
            {.type{TokenType::SeparatorDot}, .value{"."} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::istringstream iss{ "<< ident123. >>" };
        const auto expected = std::vector<Token>{
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::istringstream iss{ "<< ident123. >> ident234." };
        const auto expected = std::vector<Token>{
            {.type{TokenType::Identifier}, .value{"ident234"} },
            {.type{TokenType::SeparatorDot}, .value{"."} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::istringstream iss{ "<<<< 123 << 456 >>>>.>> ident234." };
        const auto expected = std::vector<Token>{
            {.type{TokenType::Identifier}, .value{"ident234"} },
            {.type{TokenType::SeparatorDot}, .value{"."} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::istringstream iss{ "<<<<123>>ident234<<<<123<<456>>>:>>." };
        const auto expected = std::vector<Token>{
            {.type{TokenType::Identifier}, .value{"ident234"} },
            {.type{TokenType::SeparatorDot}, .value{"."} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    //{
    //    // Error
    //    std::istringstream iss{ "<<<<123>>ident234<<<<123<<456>>>:>." };
    //    const auto expected = std::vector<Token>{
    //        {.type{TokenType::Identifier}, .value{"ident234"} },
    //    };
    //    const auto tokens = lexer.Lex(iss);
    //    assert(tokens == expected);
    //}

    {
        std::istringstream iss{ "1 < 2" };
        const auto expected = std::vector<Token>{
            {.type{TokenType::LiteralInteger}, .value{"1"} },
            {.type{TokenType::OperatorComparison}, .value{"<"} },
            {.type{TokenType::LiteralInteger}, .value{"2"} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }
}

int main()
{
    Lexer1 lexer1;
    Lexer2 lexer2;
    TestLexer(lexer1);
    TestLexer(lexer2);
}
