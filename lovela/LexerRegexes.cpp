#include "pch.h"
#include "LexerRegexes.h"

// https://stackoverflow.com/questions/399078/what-special-characters-must-be-escaped-in-regular-expressions

const std::regex& LexerRegexes::GetSeparatorRegex()
{
	static const std::regex re{ R"([()[\]{}.,:;!?/|#])" };
	return re;
}

const std::regex& LexerRegexes::GetWhitespaceRegex()
{
	static const std::regex re{ R"(\s)" };
	return re;
}

const std::regex& LexerRegexes::GetBeginLiteralNumberRegex()
{
	static const std::regex re{ R"((\d\d?|[+\-]\d|\d\.))" };
	return re;
}

const std::regex& LexerRegexes::GetBeginCommentRegex()
{
	static const std::regex re{ R"(<<)" };
	return re;
}

const std::regex& LexerRegexes::GetEndCommentRegex()
{
	static const std::regex re{ R"(>>)" };
	return re;
}

const std::regex& LexerRegexes::GetDigitRegex()
{
	static const std::regex re{ R"(\d)" };
	return re;
}

const std::regex& LexerRegexes::GetBeginStringRegex()
{
	static const std::regex re{ R"(')" };
	return re;
}

const std::regex& LexerRegexes::GetStringFieldRegex()
{
	static const std::regex re{ R"([tnr])" };
	return re;
}

const std::regex& LexerRegexes::GetBeginDecimalPartRegex()
{
	static const std::regex re{ R"(\.\d)" };
	return re;
}

const std::regex& LexerRegexes::GetBeginDecimalExponentRegex()
{
	static const std::regex re{ R"([eE][+\-\d])" };
	return re;
}
