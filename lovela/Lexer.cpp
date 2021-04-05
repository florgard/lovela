#include "pch.h"
#include "Lexer.h"

static const std::wregex separator{ LR"([\(\)\[\]\{\}\.,:;\!\?\|#])" };
static const std::wregex whitespace{ LR"(\s)" };
static const std::wregex digit{ LR"(\d)" };
static const std::wregex numberLiteral{ LR"([\+\-]?\d+)" };

Lexer::Lexer(std::wistream& charStream) noexcept : charStream(charStream >> std::noskipws)
{
}

Token Lexer::GetCurrenToken()
{
	auto token = GetToken(currentLexeme);
	currentLexeme.clear();
	state.Clear();
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
	currentLexeme.clear();
	currentLine = 1;
	currentColumn = 1;
	errors.clear();
	state.Clear();

	// Populate next and next after characters.
	GetNextCharacter();
	GetNextCharacter();

	std::vector<Token> tokens;

	while (characters[Next])
	{
		if (state.stringFieldCode)
		{
			LexStringFieldCode(tokens);
		}
		else if (state.stringLiteral)
		{
			LexLiteralString(tokens);
		}
		else if (Accept('<'))
		{
			LexParenAngleOpen(tokens);
		}
		else if (Accept('>'))
		{
			LexParenAngleClose(tokens);
		}
		else if (Accept([&] { return state.commentLevel > 0; }))
		{
			// Consume the comment.
		}
		else if (Accept('\''))
		{
			LexLiteralStringBegin(tokens);
		}
		else if (AcceptBegin(numberLiteral, 2))
		{
			LexLiteralInteger(tokens);
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

	if (state.commentLevel)
	{
		AddError(Error::Code::CommentOpen, L"A comment has not been terminated.");
	}
	else if (state.stringLiteral)
	{
		AddError(Error::Code::StringLiteralOpen, L"A string literal has not been terminated.");
	}
	else
	{
		// Get the possible token at the very end of the stream.
		auto token = GetCurrenToken();
		if (token)
		{
			co_yield DecorateToken(std::move(token));
		}

		co_yield DecorateToken({ .type = Token::Type::End });
	}
}

void Lexer::GetNextCharacter() noexcept
{
	characters[Previous] = characters[Current];
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

bool Lexer::Accept(std::function<bool()> predicate) noexcept
{
	if (predicate())
	{
		return Accept();
	}

	return false;
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

void Lexer::LexStringFieldCode(std::vector<Token>& tokens) noexcept
{
	if (Accept('}'))
	{
		if (std::iswdigit(state.stringFieldCode))
		{
			// Indexed string interpolation. Add the string literal up to this point as a token.
			tokens.emplace_back(Token{ .type = Token::Type::LiteralString, .value = currentLexeme, .outType = stringTypeName });
			currentLexeme.clear();

			// Add a string literal interpolation token with the given index.
			tokens.emplace_back(Token{ .type = Token::Type::LiteralStringInterpolation, .value = std::wstring(1, state.stringFieldCode) });
		}
		else
		{
			// Add the string field value to the string literal.
			currentLexeme += GetStringField(state.stringFieldCode);
		}
	}
	else
	{
		AddError(Error::Code::StringFieldIllformed, std::wstring(L"Ill-formed string field \"") + state.stringFieldCode + L"\".");
	}

	state.stringFieldCode = 0;
}

void Lexer::LexLiteralString(std::vector<Token>& tokens) noexcept
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

			state.stringLiteral = false;
			state.nextStringInterpolation = '1';
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
			if (state.nextStringInterpolation > '9')
			{
				AddError(Error::Code::StringInterpolationOverflow, std::wstring(L"Too many string interpolations, index out of bounds (greater than 9)."));
			}
			else
			{
				tokens.emplace_back(Token{ .type = Token::Type::LiteralStringInterpolation, .value = std::wstring(1, state.nextStringInterpolation) });
				state.nextStringInterpolation++;
			}
		}
		else if (Accept([&] { return std::iswdigit(characters[Next]) || !GetStringField(characters[Next]).empty(); }))
		{
			state.stringFieldCode = characters[Current];
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
}

void Lexer::LexLiteralInteger(std::vector<Token>& tokens) noexcept
{
	currentLexeme = characters[Current];

	while (Accept(digit, 1))
	{
		currentLexeme += characters[Current];
	}

	if (Accept('.'))
	{
		if (Accept(digit, 1))
		{
			// Accept a single decimal point in numbers. Go from integer to decimal literal.
			currentLexeme += '.';
			currentLexeme += characters[Current];

			while (Accept(digit, 1))
			{
				currentLexeme += characters[Current];
			}

			// The integer literal has ended, add it.
			tokens.emplace_back(GetCurrenToken());
		}
		else
		{
			// Full stop separator token.
			LexSeparator(tokens);
		}
	}
	else
	{
		// The integer literal has ended, add it.
		tokens.emplace_back(GetCurrenToken());
	}
}

void Lexer::LexParenAngleOpen(std::vector<Token>& tokens) noexcept
{
	if (characters[Current] == characters[Previous])
	{
		// Still opening comment.
	}
	else if (characters[Current] == characters[Next])
	{
		// Begin opening comment.

		const auto commentLevel = state.commentLevel;
		if (!commentLevel)
		{
			// Add the token before the comment.
			tokens.emplace_back(GetCurrenToken());
		}

		state.commentLevel = commentLevel + 1;
	}
	else
	{
		// Not a comment separator, add the paren to the lexeme.
		currentLexeme += characters[Current];
	}
}

void Lexer::LexParenAngleClose(std::vector<Token>&) noexcept
{
	if (characters[Current] == characters[Previous])
	{
		// Still closing comment.
	}
	else if (characters[Current] == characters[Next])
	{
		// Begin closing comment.
		state.commentLevel--;

		currentLexeme.clear();
	}
	else
	{
		// Not a comment separator, add the paren to the lexeme.
		currentLexeme += characters[Current];
	}
}

void Lexer::LexLiteralStringBegin(std::vector<Token>& tokens) noexcept
{
	// Add the token before the string literal.
	tokens.emplace_back(GetCurrenToken());

	state.stringLiteral = true;
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
