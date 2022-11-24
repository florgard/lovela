#include "pch.h"
#include "LexerRegexes.h"

// https://stackoverflow.com/questions/399078/what-special-characters-must-be-escaped-in-regular-expressions

const std::regex& LexerRegexes::GetSeparator()
{
	static const std::regex re{ R"([()[\]{}.,:;!?/|#])" };
	return re;
}

const std::regex& LexerRegexes::GetWhitespace()
{
	static const std::regex re{ R"(\s)" };
	return re;
}

const std::regex& LexerRegexes::GetBeginLiteralNumber()
{
	static const std::regex re{ R"((\d.|[+\-]\d))" };
	return re;
}

const std::regex& LexerRegexes::GetBeginComment()
{
	static const std::regex re{ R"(<<)" };
	return re;
}

const std::regex& LexerRegexes::GetEndComment()
{
	static const std::regex re{ R"(>>)" };
	return re;
}

const std::regex& LexerRegexes::GetDigit()
{
	static const std::regex re{ R"(\d)" };
	return re;
}

const std::regex& LexerRegexes::GetBeginString()
{
	static const std::regex re{ R"(')" };
	return re;
}

const std::regex& LexerRegexes::GetStringField()
{
	static const std::regex re{ R"([tnr])" };
	return re;
}

const std::regex& LexerRegexes::GetBeginDecimalPart()
{
	static const std::regex re{ R"(\.\d)" };
	return re;
}

const std::regex& LexerRegexes::GetBeginDecimalExponent()
{
	static const std::regex re{ R"([eE][+\-\d])" };
	return re;
}
