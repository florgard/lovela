#include "pch.h"
#include "LexerBase.h"

LexerBase::LexerBase(std::istream& charStream) noexcept
	: _charStream(&charStream)
{
	*_charStream >> std::noskipws;
	currentTokens.reserve(64);
}

void LexerBase::GetNextCharacter() noexcept
{
	characters[Current] = characters[Next];
	characters[Next] = characters[NextAfter];
	characters[NextAfter] = 0;
	*_charStream >> characters[NextAfter];
}

void LexerBase::AddCodeLine() noexcept
{
	sourceCodeLines.push_back(currentSourceCode.str());
	currentSourceCode.clear();
	currentLine++;
	currentTokenColumn = 1;
	nextTokenColumn = 1;
}

void LexerBase::AddToken(Token&& token) noexcept
{
	if (!token)
	{
		return;
	}

	token.error.line = currentLine;
	token.error.column = currentTokenColumn;

	currentTokenColumn = nextTokenColumn;

	currentTokens.emplace_back(std::move(token));
}

void LexerBase::AddCurrenToken() noexcept
{
	if (currentLexeme.empty())
	{
		return;
	}

	AddToken(GetToken(currentLexeme));

	currentLexeme.clear();
}

Token& LexerBase::GetCurrentToken() noexcept
{
	return currentTokens.back();
}

const Token& LexerBase::GetCurrentToken() const noexcept
{
	return currentTokens.back();
}

void LexerBase::WordBreak() noexcept
{
	AddCurrenToken();
	expectWordBreak = false;
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

bool LexerBase::Expect(char pattern) noexcept
{
	if (Accept(pattern))
	{
		return true;
	}

	AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::SyntaxError, .message = fmt::format("Unexpected character \"{}\", expected \"{}\".", characters[Next], pattern) } });
	return false;
}

bool LexerBase::Expect(LexerPatterns::Chars pattern) noexcept
{
	if (Accept(pattern))
	{
		return true;
	}

	AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::SyntaxError, .message = fmt::format("Unexpected characters \"{}{}\", expected \"{}{}\".", characters[Next], characters[NextAfter], pattern.next, pattern.nextAfter) } });
	return false;
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
