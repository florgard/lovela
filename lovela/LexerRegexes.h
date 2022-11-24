#pragma once

struct LexerRegexes
{
	const std::regex& GetWhitespace();
	const std::regex& GetSeparator();
	const std::regex& GetDigit();

	const std::regex& GetBeginComment();
	const std::regex& GetEndComment();
	const std::regex& GetBeginLiteralNumber();
	const std::regex& GetBeginDecimalPart();
	const std::regex& GetBeginDecimalExponent();
	const std::regex& GetBeginString();
	const std::regex& GetStringField();
};
