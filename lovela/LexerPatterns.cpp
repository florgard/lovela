#include "pch.h"
#include "LexerPatterns.h"

const std::regex& LexerPatterns::GetBeginLiteralNumber()
{
	static const std::regex re{ R"((\d.|[+\-]\d))" };
	return re;
}

const std::regex& LexerPatterns::GetBeginComment()
{
	static const std::regex re{ R"(<<)" };
	return re;
}

const std::regex& LexerPatterns::GetEndComment()
{
	static const std::regex re{ R"(>>)" };
	return re;
}

const std::regex& LexerPatterns::GetBeginString()
{
	static const std::regex re{ R"(')" };
	return re;
}

const std::regex& LexerPatterns::GetStringField()
{
	static const std::regex re{ R"([tnr])" };
	return re;
}

const std::regex& LexerPatterns::GetBeginDecimalPart()
{
	static const std::regex re{ R"(\.\d)" };
	return re;
}

const std::regex& LexerPatterns::GetBeginDecimalExponent()
{
	static const std::regex re{ R"([eE][+\-\d])" };
	return re;
}
