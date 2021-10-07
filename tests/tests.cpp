import LexerBase;
//import LexerFactory;
//import Parser.Node;
//import IParser;
//import ParserFactory;
//import CodeGeneratorFactory;
//import Utility;
//import Utility.StaticMap;
import <utility>;
import <string>;
import <string_view>;
import <array>;
import <vector>;
import <regex>;

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

static constexpr std::array<std::pair<int, double>, 2> values{ { {1, 1.1}, {2, 2.2} } };
static constexpr auto map = StaticMap<int, double, values.size()>{ {values} };
static_assert(map.at(1) == 1.1);
static_assert(map.at(2) == 2.2);
static_assert(map.at_or(2, 1.1) == 2.2);
static_assert(map.at_or(3, 1.1) == 1.1);

suite static_map = [] {
	"Out of bounds"_test = [] {
		expect(throws([] { if (map.at(3) == 3.3) {} }));
	};
};

int main()
{
}
