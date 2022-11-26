#include "pch.h"
#include "Lexer.h"

Lexer::Lexer(std::istream& charStream) noexcept : charStream(charStream >> std::noskipws)
{
	currentTokens.reserve(64);
}

void Lexer::AddToken(Token&& token) noexcept
{
	if (!token)
	{
		return;
	}

	token.error.line = currentLine;
	token.error.column = currentColumn;

	if (token.IsError())
	{
		token.error.message = token.value;
	}

	currentTokens.emplace_back(std::move(token));
}

void Lexer::AddCurrenToken() noexcept
{
	if (currentLexeme.empty())
	{
		return;
	}

	AddToken(GetToken(currentLexeme));

	currentLexeme.clear();
}

void Lexer::WordBreak() noexcept
{
	AddCurrenToken();
	expectWordBreak = false;
}

void Lexer::ExpectWordBreak() noexcept
{
	expectWordBreak = true;
}

bool Lexer::IsWordBreakExpected() const noexcept
{
	return expectWordBreak;
}

TokenGenerator Lexer::Lex() noexcept
{
	// Populate next and next after characters.
	GetNextCharacter();
	GetNextCharacter();

	while (characters[Next])
	{
		if (Accept(regexes.GetWhitespace(), 1))
		{
			WordBreak();
			LexWhitespace();
		}
		else if (Accept(regexes.GetSeparator(), 1))
		{
			WordBreak();
			LexSeparator();
		}
		else if (Accept(regexes.GetBeginComment(), 2))
		{
			WordBreak();
			LexComment();
		}
		else if (IsWordBreakExpected())
		{
			WordBreak();
			AddToken({ .type = Token::Type::Error, .value = std::format("Unexpected character \"{}\".", characters[Next]), .error{.code = LexerError::SyntaxError} });
		}
		else if (AcceptBegin(regexes.GetBeginLiteralNumber(), 2))
		{
			LexLiteralNumber();
			ExpectWordBreak();
		}
		else if (AcceptBegin(regexes.GetBeginString(), 1))
		{
			LexLiteralString();
			ExpectWordBreak();
		}
		else if (Accept())
		{
			currentLexeme += characters[Current];
		}

		for (auto& token : currentTokens)
		{
			co_yield token;
		}

		currentTokens.clear();
	}

	// Add the possible token at the very end of the stream.
	WordBreak();

	// Add the end token.
	AddToken({.type = Token::Type::End});

	// Add the last code line.
	AddCodeLine();

	for (auto& token : currentTokens)
	{
		co_yield token;
	}
}

void Lexer::GetNextCharacter() noexcept
{
	characters[Current] = characters[Next];
	characters[Next] = characters[NextAfter];
	characters[NextAfter] = 0;
	charStream >> characters[NextAfter];
}

void Lexer::AddCodeLine() noexcept
{
	sourceCodeLines.push_back(currentSourceCode.str());
	currentSourceCode.clear();
	currentLine++;
	currentColumn = 1;
}

bool Lexer::Accept() noexcept
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
		currentColumn++;
	}

	return true;
}

bool Lexer::Accept(char character) noexcept
{
	if (characters[Next] == character)
	{
		return Accept();
	}

	return false;
}

/// <summary>
/// Checks if the next 1 or 2 characters match the given regex.
/// </summary>
/// <param name="regex"></param>
/// <param name="length">The number of characters to match. Must be 1 or 2.</param>
/// <returns>true on match, false on mismatch or error (length out of bounds).</returns>
bool Lexer::Accept(const std::regex& regex, size_t length) noexcept
{
	if (!(length > 0 && length <= characters.size() - Next))
	{
		AddToken({ .type = Token::Type::Error, .value = "Regex match out of bounds.", .error{.code = LexerError::InternalError } });
		return false;
	}

	const auto* str = &characters[Next];
	if (std::regex_match(str, str + length, regex))
	{
		return Accept();
	}

	return false;
}

bool Lexer::AcceptBegin(char character) noexcept
{
	return currentLexeme.empty() && Accept(character);
}

bool Lexer::AcceptBegin(const std::regex& regex, size_t length) noexcept
{
	return currentLexeme.empty() && Accept(regex, length);
}

bool Lexer::Expect(char character) noexcept
{
	if (Accept(character))
	{
		return true;
	}

	AddToken({ .type = Token::Type::Error, .value = std::format("Unexpected character \"{}\", expected \"{}\".", characters[Next], character), .error{.code = LexerError::SyntaxError} });
	return false;
}

bool Lexer::Expect(const std::regex& regex, size_t length) noexcept
{
	if (Accept(regex, length))
	{
		return true;
	}

	AddToken({ .type = Token::Type::Error, .value = std::format("Unexpected character \"{}\".", characters[Next]) });
	return false;
}

void Lexer::LexLiteralString() noexcept
{
	std::string value;
	char nextStringInterpolation = '1';

	for (;;)
	{
		if (Accept('\''))
		{
			if (Accept('\''))
			{
				// Keep a single escaped quotation mark
				value += characters[Current];
			}
			else
			{
				AddToken({.type = Token::Type::LiteralString, .value = std::move(value)});
				return;
			}
		}
		else if (Accept('{'))
		{
			if (Accept('{'))
			{
				// Keep a single escaped curly bracket
				value += characters[Current];
			}
			else if (Accept('}'))
			{
				// Unindexed string interpolation. Add the string literal up to this point as a token.
				AddToken({.type = Token::Type::LiteralString, .value = std::move(value)});

				// Add a string literal interpolation token with the next free index.
				if (nextStringInterpolation > '9')
				{
					AddToken({ .type = Token::Type::Error, .value = "Too many string interpolations, index out of bounds (greater than 9).", .error{.code = LexerError::StringInterpolationOverflow} });
				}
				else
				{
					AddToken({.type = Token::Type::LiteralStringInterpolation, .value = std::string(1, nextStringInterpolation)});
					nextStringInterpolation++;
				}
			}
			else if (Accept(regexes.GetDigit(), 1))
			{
				char stringFieldCode = characters[Current];

				if (Accept('}'))
				{
					// Indexed string interpolation. Add the string literal up to this point as a token.
					AddToken({.type = Token::Type::LiteralString, .value = std::move(value)});

					// Add a string literal interpolation token with the given index.
					AddToken({.type = Token::Type::LiteralStringInterpolation, .value = std::string(1, stringFieldCode)});
				}
				else
				{
					AddToken({ .type = Token::Type::Error, .value = std::format("Ill-formed string field \"{}\".", stringFieldCode), .error{.code = LexerError::StringFieldIllformed} });
				}
			}
			else if (Accept(regexes.GetStringField(), 1))
			{
				char stringFieldCode = characters[Current];

				if (Accept('}'))
				{
					// Add the string field value to the string literal.
					value += GetStringField(stringFieldCode);
				}
				else
				{
					AddToken({ .type = Token::Type::Error, .value = std::format("Ill-formed string field \"{}\".", stringFieldCode), .error{.code = LexerError::StringFieldIllformed} });
				}
			}
			else
			{
				AddToken({ .type = Token::Type::Error, .value = std::format("Unknown string field code \"{}\".", characters[Next]),.error{.code = LexerError::StringFieldUnknown} });
			}
		}
		else if (Accept())
		{
			// Consume the string literal
			value += characters[Current];
		}
		else
		{
			AddToken({ .type = Token::Type::Error, .value = "A string literal wasn't terminated.", .error{.code = LexerError::StringLiteralOpen} });
			return;
		}
	}
}

void Lexer::LexLiteralNumber() noexcept
{
	// FIXME: Optimize with a simple state machine.

	std::string value;
	value += characters[Current];

	auto& digitRegex = regexes.GetDigit();
	while (Accept(digitRegex, 1))
	{
		value += characters[Current];
	}

	// Accept a single decimal point in numbers.

	if (Accept(regexes.GetBeginDecimalPart(), 2))
	{
		value += characters[Current];

		while (Accept(digitRegex, 1))
		{
			value += characters[Current];
		}

		if (Accept(regexes.GetBeginDecimalExponent(), 2))
		{
			value += characters[Current];

			if (!Accept(regexes.GetBeginLiteralNumber(), 2))
			{
				AddToken({ .type = Token::Type::Error, .value = "Ill-formed literal decimal number.", .error{.code = LexerError::StringLiteralOpen} });
				return;
			}
			
			value += characters[Current];

			while (Accept(digitRegex, 1))
			{
				value += characters[Current];
			}
		}

		AddToken({.type = Token::Type::LiteralDecimal, .value = std::move(value)});
	}
	else
	{
		AddToken({.type = Token::Type::LiteralInteger, .value = std::move(value)});
	}
}

void Lexer::LexComment() noexcept
{
	while (Accept('<'));

	int commentLevel = 1;

	for (;;)
	{
		if (Accept(regexes.GetEndComment(), 2))
		{
			while (Accept('>'));

			commentLevel--;

			if (!commentLevel)
			{
				return;
			}
		}
		else if (Accept(regexes.GetBeginComment(), 2))
		{
			// Nested comment.

			while (Accept('<'));

			commentLevel++;
		}
		else if (Accept())
		{
			// Consume the comment.
		}
		else
		{
			AddToken({ .type = Token::Type::Error, .value = "A comment wasn't terminated.", .error{.code = LexerError::CommentOpen} });
			return;
		}
	}
}

void Lexer::LexSeparator() noexcept
{
	AddToken(GetToken(characters[Current]));
}

void Lexer::LexWhitespace() noexcept
{
}
