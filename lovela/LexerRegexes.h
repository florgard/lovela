#pragma once

class LexerRegexes
{
	// https://stackoverflow.com/questions/399078/what-special-characters-must-be-escaped-in-regular-expressions

public:
	static const std::regex& GetSeparatorRegex()
	{
		static const std::regex re{ R"([()[\]{}.,:;!?/|#])" };
		return re;
	}

	static const std::regex& GetWhitespaceRegex()
	{
		static const std::regex re{ R"(\s)" };
		return re;
	}

	static const std::regex& GetBeginLiteralNumberRegex()
	{
		static const std::regex re{ R"([+\-]?\d+)" };
		return re;
	}

	static const std::regex& GetBeginCommentRegex()
	{
		static const std::regex re{ R"(<<)" };
		return re;
	}

	static const std::regex& GetEndCommentRegex()
	{
		static const std::regex re{ R"(>>)" };
		return re;
	}

	static const std::regex& GetDigitRegex()
	{
		static const std::regex re{ R"(\d)" };
		return re;
	}

	static const std::regex& GetStringFieldRegex()
	{
		static const std::regex re{ R"([tnr])" };
		return re;
	}

	static const std::regex& GetDecimalPartRegex()
	{
		static const std::regex re{ R"(\.\d)" };
		return re;
	}

	static const std::regex& GetFirstCharRegex()
	{
		static const std::regex re{ R"([\d+.])" };
		return re;
	}

	static const std::regex& GetFollowingCharsRegex()
	{
		static const std::regex re{ R"([\d#])" };
		return re;
	}
};
