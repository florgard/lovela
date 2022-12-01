#include "pch.h"
#include "TestingBase.h"
#include "../lovela/LexerPatterns.h"

using namespace boost::ut;

suite regex_tests = [] {
	"separator"_test = [] {
		LexerPatterns regexes;
		auto& re = regexes.separator.regex;

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
		expect(!std::regex_match("%", re));
	};
};
