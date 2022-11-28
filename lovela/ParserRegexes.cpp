#include "pch.h"
#include "ParserRegexes.h"

const std::regex& ParserRegexes::GetLiteralDecimal() const
{
	static const std::regex re{ R"([+\-]?(\d+)\.(\d+)([eE][+\-]?\d+)?)" };
	return re;
}

const std::regex& ParserRegexes::GetLiteralIntegerTrimZeros() const
{
	static const std::regex re{ R"(0*(\d+?)0*)" };
	return re;
}
