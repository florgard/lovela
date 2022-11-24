#pragma once

class LexerRegexes
{
public:
	static const std::regex& GetSeparatorRegex();
	static const std::regex& GetWhitespaceRegex();

	static const std::regex& GetBeginCommentRegex();
	static const std::regex& GetEndCommentRegex();
	static const std::regex& GetBeginLiteralNumberRegex();
	static const std::regex& GetDigitRegex();
	static const std::regex& GetBeginDecimalPartRegex();
	static const std::regex& GetBeginDecimalExponentRegex();
	static const std::regex& GetStringFieldRegex();
};
