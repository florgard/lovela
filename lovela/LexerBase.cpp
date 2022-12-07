#include "pch.h"
#include "LexerBase.h"

void LexerBase::GetNextCharacter() noexcept
{
	characters[Current] = characters[Next];
	characters[Next] = characters[NextAfter];

	if (IsDone())
	{
		characters[NextAfter] = 0;
	}
	else
	{
		characters[NextAfter] = GetNext();
		Advance();
	}
}

void LexerBase::AddCodeLine() noexcept
{
	sourceCodeLines.push_back(currentSourceCode.str());
	currentSourceCode.clear();
	currentLine++;
	currentTokenColumn = 1;
	nextTokenColumn = 1;
}

Token LexerBase::AddToken(Token token) noexcept
{
	if (token)
	{
		token.error.line = currentLine;
		token.error.column = currentTokenColumn;

		currentTokenColumn = nextTokenColumn;
	}

	return token;
}

Token LexerBase::AddCurrenToken() noexcept
{
	Token token{};

	if (!currentLexeme.empty())
	{
		token = GetToken(currentLexeme);

		currentLexeme.clear();
	}

	return token;
}

Token LexerBase::WordBreak() noexcept
{
	expectWordBreak = false;

	return AddCurrenToken();
}

void LexerBase::ExpectWordBreak() noexcept
{
	expectWordBreak = true;
}

bool LexerBase::IsWordBreakExpected() const noexcept
{
	return expectWordBreak;
}

bool LexerBase::Accept() noexcept
{
	GetNextCharacter();

	const auto currentChar = GetCharacter(Current);
	if (!currentChar)
	{
		return false;
	}

	if (currentChar == LexerPatterns::separatorNewLine)
	{
		AddCodeLine();
	}
	else
	{
		currentSourceCode << currentChar;
		nextTokenColumn++;
	}

	return true;
}

bool LexerBase::Accept(char pattern) noexcept
{
	if (GetCharacter(Next) == pattern)
	{
		return Accept();
	}

	return false;
}

bool LexerBase::Accept(LexerPatterns::Chars pattern) noexcept
{
	if (GetCharacter(Next) == pattern.next && GetCharacter(NextAfter) == pattern.nextAfter)
	{
		return Accept();
	}

	return false;
}

bool LexerBase::AcceptBegin(char pattern) noexcept
{
	return currentLexeme.empty() && Accept(pattern);
}

bool LexerBase::AcceptBegin(LexerPatterns::Chars pattern) noexcept
{
	return currentLexeme.empty() && Accept(pattern);
}

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
