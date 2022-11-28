#pragma once

struct ParserRegexes
{
	const std::regex& GetLiteralDecimal() const;
	const std::regex& GetLiteralIntegerTrimZeros() const;
};
