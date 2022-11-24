#include "pch.h"
#include "Lexer.h"

Lexer::Lexer(std::istream& charStream) noexcept : charStream(charStream >> std::noskipws)
{
}

void Lexer::AddCurrenToken() noexcept
{
	if (currentLexeme.empty())
	{
		return;
	}

	auto token = GetToken(currentLexeme);
	if (token)
	{
		token.line = currentLine;
		token.column = currentColumn;
		token.code = std::string(currentCode.begin(), currentCode.end());
		currentTokens.emplace_back(std::move(token));
	}

	currentLexeme.clear();
}

TokenGenerator Lexer::Lex() noexcept
{
	// Populate next and next after characters.
	GetNextCharacter();
	GetNextCharacter();

	currentTokens.reserve(64);
	bool expectWordBreak = false;

	while (characters[Next])
	{
		if (Accept(regexes.GetWhitespace(), 1))
		{
			expectWordBreak = false;
			AddCurrenToken();
			LexWhitespace();
		}
		else if (Accept(regexes.GetSeparator(), 1))
		{
			expectWordBreak = false;
			AddCurrenToken();
			LexSeparator();
		}
		else if (Accept(regexes.GetBeginComment(), 2))
		{
			expectWordBreak = false;
			AddCurrenToken();
			LexComment();
		}
		else if (expectWordBreak)
		{
			expectWordBreak = false;
			const auto message = std::format("Unexpected character \"{}\".", characters[Next]);
			AddError(Error::Code::SyntaxError, message);
			currentTokens.emplace_back(Token{ .type = Token::Type::Error, .value = message });
		}
		else if (AcceptBegin(regexes.GetBeginLiteralNumber(), 2))
		{
			LexLiteralNumber();
			expectWordBreak = true;
		}
		else if (AcceptBegin(regexes.GetBeginString(), 1))
		{
			LexLiteralString();
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
	AddCurrenToken();

	// Add the end token.
	currentTokens.emplace_back(Token{ .type = Token::Type::End });

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

bool Lexer::Accept() noexcept
{
	GetNextCharacter();

	if (!characters[Current])
	{
		return false;
	}

	currentColumn++;

	static constexpr size_t codeSampleCharacterCount = 20;
	currentCode.push_back(characters[Current]);
	while (currentCode.size() > codeSampleCharacterCount)
	{
		currentCode.pop_front();
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
		AddError(Error::Code::InternalError, "Regex match out of bounds.");
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

	AddError(Error::Code::SyntaxError, std::string("Unexpected character \"") + characters[Next] + "\", expected \"" + character + " \".");
	return false;
}

bool Lexer::Expect(const std::regex& regex, size_t length) noexcept
{
	if (Accept(regex, length))
	{
		return true;
	}

	AddError(Error::Code::SyntaxError, std::string("Unexpected character \"") + characters[Next] + " \".");
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
				currentTokens.emplace_back(Token{ .type = Token::Type::LiteralString, .value = std::move(value) });
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
				currentTokens.emplace_back(Token{ .type = Token::Type::LiteralString, .value = std::move(value) });

				// Add a string literal interpolation token with the next free index.
				if (nextStringInterpolation > '9')
				{
					AddError(Error::Code::StringInterpolationOverflow, std::string("Too many string interpolations, index out of bounds (greater than 9)."));
				}
				else
				{
					currentTokens.emplace_back(Token{ .type = Token::Type::LiteralStringInterpolation, .value = std::string(1, nextStringInterpolation) });
					nextStringInterpolation++;
				}
			}
			else if (Accept(regexes.GetDigit(), 1))
			{
				char stringFieldCode = characters[Current];

				if (Accept('}'))
				{
					// Indexed string interpolation. Add the string literal up to this point as a token.
					currentTokens.emplace_back(Token{ .type = Token::Type::LiteralString, .value = std::move(value) });

					// Add a string literal interpolation token with the given index.
					currentTokens.emplace_back(Token{ .type = Token::Type::LiteralStringInterpolation, .value = std::string(1, stringFieldCode) });
				}
				else
				{
					AddError(Error::Code::StringFieldIllformed, std::string("Ill-formed string field \"") + stringFieldCode + "\".");
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
					AddError(Error::Code::StringFieldIllformed, std::string("Ill-formed string field \"") + stringFieldCode + "\".");
				}
			}
			else
			{
				AddError(Error::Code::StringFieldUnknown, std::string("Unknown string field code \"") + characters[Next] + "\".");
			}
		}
		else if (Accept())
		{
			// Consume the string literal
			value += characters[Current];
		}
		else
		{
			AddError(Error::Code::StringLiteralOpen, "A string literal wasn't terminated.");
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
				AddError(Error::Code::StringLiteralOpen, "Ill-formed literal decimal number.");
				currentTokens.emplace_back(Token{ .type = Token::Type::Error, .value = "Ill-formed literal decimal number." });
				return;
			}
			
			value += characters[Current];

			while (Accept(digitRegex, 1))
			{
				value += characters[Current];
			}
		}

		currentTokens.emplace_back(Token{ .type = Token::Type::LiteralDecimal, .value = std::move(value) });
	}
	else
	{
		currentTokens.emplace_back(Token{ .type = Token::Type::LiteralInteger, .value = std::move(value) });
	}
}

void Lexer::LexComment() noexcept
{
	while (Accept('<'))
	{
	}

	int commentLevel = 1;

	for (;;)
	{
		if (Accept(regexes.GetEndComment(), 2))
		{
			while (Accept('>'))
			{
			}

			commentLevel--;

			if (!commentLevel)
			{
				return;
			}
		}
		else if (Accept(regexes.GetBeginComment(), 2))
		{
			// Nested comment.

			while (Accept('<'))
			{
			}

			commentLevel++;
		}
		else if (Accept())
		{
			// Consume the comment.
		}
		else
		{
			AddError(Error::Code::CommentOpen, "A comment wasn't terminated.");
			currentTokens.emplace_back(Token{ .type = Token::Type::Error, .value = "A comment wasn't terminated." });
			return;
		}
	}
}

void Lexer::LexSeparator() noexcept
{
	// Add the separator token.
	currentLexeme = characters[Current];
	AddCurrenToken();
}

void Lexer::LexWhitespace() noexcept
{
	if (characters[Current] == '\n')
	{
		currentLine++;
		currentColumn = 1;
	}
}
