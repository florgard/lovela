#pragma once

struct LexerRegexes
{
	const std::regex& GetWhitespaceRegex();
	const std::regex& GetSeparatorRegex();
	const std::regex& GetDigitRegex();

	const std::regex& GetBeginCommentRegex();
	const std::regex& GetEndCommentRegex();
	const std::regex& GetBeginLiteralNumberRegex();
	const std::regex& GetBeginDecimalPartRegex();
	const std::regex& GetBeginDecimalExponentRegex();
	const std::regex& GetBeginStringRegex();
	const std::regex& GetStringFieldRegex();
};
