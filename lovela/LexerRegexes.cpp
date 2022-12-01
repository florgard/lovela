#include "pch.h"
#include "LexerRegexes.h"

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
