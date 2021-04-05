#include "pch.h"
#include "Lexer.h"

static const std::wregex separator{ LR"([\(\)\[\]\{\}\.,:;\!\?\|#])" };
static const std::wregex whitespace{ LR"(\s)" };
static const std::wregex digit{ LR"(\d)" };
static const std::wregex literalNumber{ LR"([\+\-]?\d+)" };
static const std::wregex beginComment{ LR"(<<)" };
static const std::wregex endComment{ LR"(>>)" };
static const std::wregex stringField{ LR"([tnr])" };

Lexer::Lexer(std::wistream& charStream) noexcept : charStream(charStream >> std::noskipws)
{
}

Token Lexer::GetCurrenToken()
{
	auto token = GetToken(currentLexeme);
	currentLexeme.clear();
	return token;
}

Token Lexer::DecorateToken(Token token) const
{
	if (token)
	{
		token.line = currentLine;
		token.column = currentColumn;
		token.code = std::wstring(currentCode.begin(), currentCode.end());
	}
	return token;
}

TokenGenerator Lexer::Lex() noexcept
{
	// Populate next and next after characters.
	GetNextCharacter();
	GetNextCharacter();

	std::vector<Token> tokens;

	while (characters[Next])
	{
		if (Accept(beginComment, 2))
		{
			LexComment(tokens);
		}
		else if (Accept('\''))
		{
			LexLiteralString(tokens);
		}
		else if (AcceptBegin(literalNumber, 2))
		{
			LexLiteralNumber(tokens);
		}
		else if (AcceptBegin('#'))
		{
			LexPrimitiveType(tokens);
		}
		else if (Accept(separator, 1))
		{
			LexSeparator(tokens);
		}
		else if (Accept(whitespace, 1))
		{
			LexWhitespace(tokens);
		}
		else if (Accept())
		{
			currentLexeme += characters[Current];
		}

		for (auto& token : tokens)
		{
			if (token)
			{
				co_yield DecorateToken(std::move(token));
			}
		}

		tokens.clear();
	}

	// Get the possible token at the very end of the stream.
	auto token = GetCurrenToken();
	if (token)
	{
		co_yield DecorateToken(std::move(token));
	}

	co_yield DecorateToken({ .type = Token::Type::End });
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

bool Lexer::Accept(wchar_t character) noexcept
{
	if (characters[Next] == character)
	{
		return Accept();
	}

	return false;
}

bool Lexer::Accept(const std::wregex& regex, size_t length) noexcept
{
	assert(length == 1 || length == 2);
	std::wstring_view str(&characters[Next], length);
	if (std::regex_match(str.begin(), str.end(), regex))
	{
		return Accept();
	}

	return false;
}

bool Lexer::AcceptBegin(wchar_t character) noexcept
{
	return currentLexeme.empty() && Accept(character);
}

bool Lexer::AcceptBegin(const std::wregex& regex, size_t length) noexcept
{
	return currentLexeme.empty() && Accept(regex, length);
}

bool Lexer::Expect(wchar_t character) noexcept
{
	if (Accept(character))
	{
		return true;
	}

	AddError(Error::Code::SyntaxError, std::wstring(L"Unexpected character \"") + characters[Next] + L"\", expected \"" + character + L" \".");
	return false;
}

bool Lexer::Expect(const std::wregex& regex, size_t length) noexcept
{
	if (Accept(regex, length))
	{
		return true;
	}

	AddError(Error::Code::SyntaxError, std::wstring(L"Unexpected character \"") + characters[Next] + L" \".");
	return false;
}

void Lexer::LexLiteralString(std::vector<Token>& tokens) noexcept
{
	wchar_t nextStringInterpolation = '1';

	for (;;)
	{
		if (Accept('\''))
		{
			if (Accept('\''))
			{
				// Keep a single escaped quotation mark
				currentLexeme += characters[Current];
			}
			else
			{
				tokens.emplace_back(Token{ .type = Token::Type::LiteralString, .value = currentLexeme, .outType = stringTypeName });
				currentLexeme.clear();
				return;
			}
		}
		else if (Accept('{'))
		{
			if (Accept('{'))
			{
				// Keep a single escaped curly bracket
				currentLexeme += characters[Current];
			}
			else if (Accept('}'))
			{
				// Unindexed string interpolation. Add the string literal up to this point as a token.
				tokens.emplace_back(Token{ .type = Token::Type::LiteralString, .value = currentLexeme, .outType = stringTypeName });
				currentLexeme.clear();

				// Add a string literal interpolation token with the next free index.
				if (nextStringInterpolation > '9')
				{
					AddError(Error::Code::StringInterpolationOverflow, std::wstring(L"Too many string interpolations, index out of bounds (greater than 9)."));
				}
				else
				{
					tokens.emplace_back(Token{ .type = Token::Type::LiteralStringInterpolation, .value = std::wstring(1, nextStringInterpolation) });
					nextStringInterpolation++;
				}
			}
			else if (Accept(digit, 1))
			{
				wchar_t stringFieldCode = characters[Current];

				if (Accept('}'))
				{
					// Indexed string interpolation. Add the string literal up to this point as a token.
					tokens.emplace_back(Token{ .type = Token::Type::LiteralString, .value = currentLexeme, .outType = stringTypeName });
					currentLexeme.clear();

					// Add a string literal interpolation token with the given index.
					tokens.emplace_back(Token{ .type = Token::Type::LiteralStringInterpolation, .value = std::wstring(1, stringFieldCode) });
				}
				else
				{
					AddError(Error::Code::StringFieldIllformed, std::wstring(L"Ill-formed string field \"") + stringFieldCode + L"\".");
				}
			}
			else if (Accept(stringField, 1))
			{
				wchar_t stringFieldCode = characters[Current];

				if (Accept('}'))
				{
					// Add the string field value to the string literal.
					currentLexeme += GetStringField(stringFieldCode);
				}
				else
				{
					AddError(Error::Code::StringFieldIllformed, std::wstring(L"Ill-formed string field \"") + stringFieldCode + L"\".");
				}
			}
			else
			{
				AddError(Error::Code::StringFieldUnknown, std::wstring(L"Unknown string field code \"") + characters[Next] + L"\".");
			}
		}
		else if (Accept())
		{
			// Consume the string literal
			currentLexeme += characters[Current];
		}
		else
		{
			AddError(Error::Code::StringLiteralOpen, L"A string literal wasn't terminated.");
			currentLexeme.clear();
			return;
		}
	}
}

void Lexer::LexLiteralNumber(std::vector<Token>& tokens) noexcept
{
	currentLexeme = characters[Current];

	while (Accept(digit, 1))
	{
		currentLexeme += characters[Current];
	}

	static const std::wregex decimalPart{ LR"(\.\d)" };

	if (Accept(decimalPart, 2))
	{
		// Accept a single decimal point in numbers. Go from integer to decimal literal.
		currentLexeme += characters[Current];

		while (Accept(digit, 1))
		{
			currentLexeme += characters[Current];
		}
	}

	// The numeric literal has ended, add it.
	tokens.emplace_back(GetCurrenToken());
}

void Lexer::LexComment(std::vector<Token>& tokens) noexcept
{
	// Add the token before the comment.
	tokens.emplace_back(GetCurrenToken());

	while (Accept('<'))
	{
	}

	int commentLevel = 1;

	for (;;)
	{
		if (Accept(endComment, 2))
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
		else if (Accept(beginComment, 2))
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
			AddError(Error::Code::CommentOpen, L"A comment wan't terminated.");
			return;
		}
	}
}

void Lexer::LexSeparator(std::vector<Token>& tokens) noexcept
{
	// Add the token before the separator.
	tokens.emplace_back(GetCurrenToken());

	// Add the separator token.
	currentLexeme = characters[Current];
	tokens.emplace_back(GetCurrenToken());
}

void Lexer::LexWhitespace(std::vector<Token>& tokens) noexcept
{
	// Add the token before the whitespace.
	tokens.emplace_back(GetCurrenToken());

	if (characters[Current] == '\n')
	{
		currentLine++;
		currentColumn = 1;
	}
}

void Lexer::LexPrimitiveType(std::vector<Token>& tokens) noexcept
{
	static const std::wregex firstChar{LR"([\d\+\.])"};
	static const std::wregex followingChars{ LR"([\d#])" };

	std::wstring lexeme;
	lexeme += characters[Current];

	if (!Expect(firstChar, 1))
	{
		AddError(Error::Code::SyntaxError, L"Invalid primitive type.");
		return;
	}

	lexeme += characters[Current];

	while (Accept(followingChars, 1))
	{
		lexeme += characters[Current];
	}

	tokens.emplace_back(Token{ .type = Token::Type::PrimitiveType, .value = std::move(lexeme) });
}
