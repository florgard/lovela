#include <iostream>
#include <sstream>
#include <cassert>
#include "Lexer.h"

void TestLexer(ILexer& lexer)
{
    {
        std::wistringstream iss{ L"" };
        const auto expected = std::vector<Token>{
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::wistringstream iss{ L"." };
        const auto expected = std::vector<Token>{
            {.type{TokenType::SeparatorDot}, .value{L"."} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::wistringstream iss{ L"abc" };
        const auto expected = std::vector<Token>{
            {.type{TokenType::Identifier}, .value{L"abc"} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::wistringstream iss{ L"\u65E5\u672C" };
        const auto expected = std::vector<Token>{
            {.type{TokenType::Identifier}, .value{L"\u65E5\u672C"} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    //{
    //    std::wistringstream iss{ L"\u0061\u0300" };
    //    const auto expected = std::vector<Token>{
    //        {.type{TokenType::Identifier}, .value{L"\u0061\u0300"} },
    //    };
    //    const auto tokens = lexer.Lex(iss);
    //    assert(tokens == expected);
    //}

    {
        std::wistringstream iss{ L"123" };
        const auto expected = std::vector<Token>{
            {.type{TokenType::LiteralInteger}, .value{L"123"} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::wistringstream iss{ L"123.456" };
        const auto expected = std::vector<Token>{
            {.type{TokenType::LiteralDecimal}, .value{L"123.456"} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::wistringstream iss{ L"''" };
        const auto expected = std::vector<Token>{
            {.type{TokenType::LiteralString}, .value{L""} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::wistringstream iss{ L"'abc'" };
        const auto expected = std::vector<Token>{
            {.type{TokenType::LiteralString}, .value{L"abc"} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::wistringstream iss{ L"'ab''c'" };
        const auto expected = std::vector<Token>{
            {.type{TokenType::LiteralString}, .value{L"ab'c"} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::wistringstream iss{ L"func: 123 ." };
        const auto expected = std::vector<Token>{
            {.type{TokenType::Identifier}, .value{L"func"} },
            {.type{TokenType::OperatorColon}, .value{L":"} },
            {.type{TokenType::LiteralInteger}, .value{L"123"} },
            {.type{TokenType::SeparatorDot}, .value{L"."} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::wistringstream iss{ L"func: 123 ." };
        const auto expected = std::vector<Token>{
            {.type{TokenType::Identifier}, .value{L"func"} },
            {.type{TokenType::OperatorColon}, .value{L":"} },
            {.type{TokenType::LiteralInteger}, .value{L"123"} },
            {.type{TokenType::SeparatorDot}, .value{L"."} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::wistringstream iss{ L"func : 123.4 ." };
        const auto expected = std::vector<Token>{
            {.type{TokenType::Identifier}, .value{L"func"} },
            {.type{TokenType::OperatorColon}, .value{L":"} },
            {.type{TokenType::LiteralDecimal}, .value{L"123.4"} },
            {.type{TokenType::SeparatorDot}, .value{L"."} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::wistringstream iss{ L"\r\nfunc44: (123.4)." };
        const auto expected = std::vector<Token>{
            {.type{TokenType::Identifier}, .value{L"func44"} },
            {.type{TokenType::OperatorColon}, .value{L":"} },
            {.type{TokenType::ParenRoundOpen}, .value{L"("} },
            {.type{TokenType::LiteralDecimal}, .value{L"123.4"} },
            {.type{TokenType::ParenRoundClose}, .value{L")"} },
            {.type{TokenType::SeparatorDot}, .value{L"."} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::wistringstream iss{ L"ident123." };
        const auto expected = std::vector<Token>{
            {.type{TokenType::Identifier}, .value{L"ident123"} },
            {.type{TokenType::SeparatorDot}, .value{L"."} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::wistringstream iss{ L"<< ident123. >>" };
        const auto expected = std::vector<Token>{
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::wistringstream iss{ L"<< ident123. >> ident234." };
        const auto expected = std::vector<Token>{
            {.type{TokenType::Identifier}, .value{L"ident234"} },
            {.type{TokenType::SeparatorDot}, .value{L"."} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::wistringstream iss{ L"<<<< 123 << 456 >>>>.>> ident234." };
        const auto expected = std::vector<Token>{
            {.type{TokenType::Identifier}, .value{L"ident234"} },
            {.type{TokenType::SeparatorDot}, .value{L"."} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    {
        std::wistringstream iss{ L"<<<<123>>ident234<<<<123<<456>>>:>>." };
        const auto expected = std::vector<Token>{
            {.type{TokenType::Identifier}, .value{L"ident234"} },
            {.type{TokenType::SeparatorDot}, .value{L"."} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }

    //{
    //    // Error
    //    std::wistringstream iss{ L"<<<<123>>ident234<<<<123<<456>>>:>." };
    //    const auto expected = std::vector<Token>{
    //        {.type{TokenType::Identifier}, .value{L"ident234"} },
    //    };
    //    const auto tokens = lexer.Lex(iss);
    //    assert(tokens == expected);
    //}

    {
        std::wistringstream iss{ L"1 < 2" };
        const auto expected = std::vector<Token>{
            {.type{TokenType::LiteralInteger}, .value{L"1"} },
            {.type{TokenType::OperatorComparison}, .value{L"<"} },
            {.type{TokenType::LiteralInteger}, .value{L"2"} },
        };
        const auto tokens = lexer.Lex(iss);
        assert(tokens == expected);
    }
}

int main()
{
    Lexer1 lexer1;
    Lexer2 lexer2;
    //TestLexer(lexer1);
    TestLexer(lexer2);
}