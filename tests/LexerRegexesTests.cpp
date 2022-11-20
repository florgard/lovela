#include "pch.h"
#include "TestingBase.h"
#include "../lovela/LexerRegexes.h"

#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp"
using namespace boost::ut;

suite regex_tests = [] {
	"separator"_test = [] {
		auto& re = LexerRegexes::GetSeparatorRegex();

		expect(std::regex_match("(", re));
		expect(std::regex_match(")", re));
		expect(std::regex_match("[", re));
		expect(std::regex_match("]", re));
		expect(std::regex_match("{", re));
		expect(std::regex_match("}", re));
		expect(std::regex_match(".", re));
		expect(std::regex_match(",", re));
		expect(std::regex_match(":", re));
		expect(std::regex_match(";", re));
		expect(std::regex_match("!", re));
		expect(std::regex_match("?", re));
		expect(std::regex_match("|", re));
		expect(std::regex_match("#", re));

		expect(!std::regex_match("\\s", re));
		expect(!std::regex_match("\\d", re));

		expect(!std::regex_match("+", re));
		expect(!std::regex_match("-", re));
		expect(!std::regex_match("*", re));
		expect(!std::regex_match("/", re));
		expect(!std::regex_match("%", re));
	};
};