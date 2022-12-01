#include "pch.h"
#include "LexerBase.h"

LexerBase::LexerBase(std::istream& charStream) noexcept
	: charStream(charStream >> std::noskipws)
{
	currentTokens.reserve(64);
}

void LexerBase::GetNextCharacter() noexcept
{
	characters[Current] = characters[Next];
	characters[Next] = characters[NextAfter];
	characters[NextAfter] = 0;
	charStream >> characters[NextAfter];
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

	if (!characters[Current])
	{
		return false;
	}

	if (characters[Current] == '\n')
	{
		AddCodeLine();
	}
	else
	{
		currentSourceCode << characters[Current];
		nextTokenColumn++;
	}

	return true;
}

bool LexerBase::Accept(char pattern) noexcept
{
	if (characters[Next] == pattern)
	{
		return Accept();
	}

	return false;
}

bool LexerBase::Accept(LexerPatterns::Chars pattern) noexcept
{
	if (characters[Next] == pattern.first && characters[NextAfter] == pattern.second)
	{
		return Accept();
	}

	return false;
}

/// <summary>
/// Checks if the next 1 or 2 characters match the given regex.
/// </summary>
/// <param name="pattern">The pattern containing the regex and length to match.</param>
/// <returns>true on match, false on mismatch or error (length out of bounds).</returns>
bool LexerBase::Accept(const LexerPatterns::Regex& pattern) noexcept
{
	if (!(pattern.length > 0 && pattern.length <= characters.size() - Next))
	{
		AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::InternalError, .message = "Pattern match out of bounds." } });
		return false;
	}

	const auto* str = &characters[Next];
	if (std::regex_match(str, str + pattern.length, pattern.regex))
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

bool LexerBase::AcceptBegin(const LexerPatterns::Regex& pattern) noexcept
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

	AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::SyntaxError, .message = fmt::format("Unexpected characters \"{}{}\", expected \"{}{}\".", characters[Next], characters[NextAfter], pattern.first, pattern.second) } });
	return false;
}

bool LexerBase::Expect(const LexerPatterns::Regex& pattern) noexcept
{
	if (Accept(pattern))
	{
		return true;
	}

	AddToken({ .type = Token::Type::Error, .value = fmt::format("Unexpected character \"{}\".", characters[Next]) });
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
