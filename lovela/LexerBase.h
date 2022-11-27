#pragma once
#include "ILexer.h"

class LexerBase : public ILexer
{
public:
	void PrintErrorSourceCode(std::ostream& stream, const Token& token) noexcept override;

protected:
	size_t currentLine = 1;
	size_t currentTokenColumn = 1;
	size_t nextTokenColumn = 1;
	std::ostringstream currentSourceCode;

	size_t firstSourceCodeLine = 1;
	std::deque<std::string> sourceCodeLines;

	struct Color
	{
		std::string_view none = "\033[0m";
		std::string_view fail = "\033[97m\033[41m";
		std::string_view code = "\033[96m";
	} color;
};
