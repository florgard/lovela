#include "pch.h"
#include "LexerBase.h"

void LexerBase::PrintErrorSourceCode(std::ostream& stream, const Token& token) noexcept
{
	const auto line = token.error.line;
	const auto column = token.error.column;

	if (line < firstSourceCodeLine || line > firstSourceCodeLine + sourceCodeLines.size() - 1)
	{
		stream << "No source code available for line " << line << ".\n";
		return;
	}

	const auto& sourceCode = sourceCodeLines[line - firstSourceCodeLine];

	const auto length = sourceCode.length();
	const auto begin = std::min(column - 1, length);
	auto end = std::min(begin + token.value.length(), length);
	const auto count = end - begin;

	stream << '(' << line << ':' << column << ") " << color.code << sourceCode.substr(0, begin) << color.fail;

	if (begin < end)
	{
		stream << sourceCode.substr(begin, count);
	}
	else if (end < length)
	{
		// Highlight the character after if the token is empty.
		++end;
		stream << sourceCode.substr(begin, 1);
	}
	else
	{
		// Add highlighted padding at the end of the line.
		stream << ' ';
	}

	stream << color.none << color.code;

	// Write the remaining part of the line, if any.
	if (end < length)
	{
		stream << sourceCode.substr(end);
	}

	stream << color.none << '\n';

	// Drop all lines before the requested one to avoid keeping a lot of lines in memory.
	// It's assumed that errors are always printed in a forward order.

	while (firstSourceCodeLine < line)
	{
		sourceCodeLines.pop_front();
		++firstSourceCodeLine;
	}
}
