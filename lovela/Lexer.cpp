#include "pch.h"
#include "Lexer.h"

Lexer::Lexer(std::wistream& charStream) noexcept : charStream(charStream >> std::noskipws)
{
}

Token Lexer::GetCurrentLexemeToken()
{
	auto token = GetToken(currentLexeme);
	currentLexeme.clear();
	state.Clear();
	return token;
}

Token Lexer::GetCurrentCharToken()
{
	return GetToken(currentChar);
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
	static constexpr std::wstring_view separators{ L"()[]{}.,:;!?|#" };

	currentLexeme.clear();
	currentLine = 1;
	currentColumn = 1;
	errors.clear();
	state.Clear();

	charStream >> nextChar;

	std::vector<Token> tokens;

	while (nextChar)
	{
		tokens.clear();
		
		if (state.stringFieldCode)
		{
			LexStringFieldCode(tokens);
		}
		else if (state.stringLiteral)
		{
			LexLiteralString(tokens);
		}
		else if (state.integerLiteral)
		{
			LexLiteralInteger(tokens);
		}
		else if (Accept('<'))
		{
			LexParenAngleOpen(tokens);
		}
		else if (Accept('>'))
		{
			LexParenAngleClose(tokens);
		}
		else if (state.commentLevel)
		{
			// Consume the comment
			Accept();
		}
		else if (Accept('\''))
		{
			LexLiteralStringBegin(tokens);
		}
		else if (std::iswdigit(nextChar) && currentLexeme.empty())
		{
			Accept();
			LexLiteralIntegerBegin(tokens);
		}
		else if (separators.find(nextChar) != separators.npos)
		{
			Accept();
			LexSeparator(tokens);
		}
		else if (std::iswspace(nextChar))
		{
			Accept();
			LexWhitespace(tokens);
		}
		else if (Accept())
		{
			currentLexeme += currentChar;
		}

		for (auto& token : tokens)
		{
			if (token)
			{
				co_yield DecorateToken(std::move(token));
			}
		}
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
		auto token = GetCurrentLexemeToken();
		if (token)
		{
			co_yield DecorateToken(std::move(token));
		}

		co_yield DecorateToken({ .type = Token::Type::End });
	}
}

bool Lexer::Accept()
{
	currentColumn++;

	if (nextChar)
	{
		previousChar = currentChar;
		currentChar = nextChar;
		nextChar = 0;
		charStream >> nextChar;

		static constexpr size_t codeSampleCharacters = 20;
		currentCode.push_back(currentChar);
		while (currentCode.size() > codeSampleCharacters)
		{
			currentCode.pop_front();
		}

		return true;
	}

	return false;
}

bool Lexer::Peek(wchar_t token)
{
	return token == nextChar;
}

bool Lexer::Accept(wchar_t token)
{
	if (Peek(token))
	{
		return Accept();
	}

	return false;
}

bool Lexer::Accept(const std::vector<wchar_t>& tokens)
{
	if (std::find(tokens.begin(), tokens.end(), nextChar) != tokens.end())
	{
		return Accept();
	}

	return false;
}

void Lexer::Expect(wchar_t token)
{
	if (!Accept(token))
	{
		AddError(Error::Code::SyntaxError, std::wstring(L"Unexpected character \"") + nextChar + L"\", expected \"" + token + L" \".");
	}
}

void Lexer::Expect(const std::vector<wchar_t>& tokens)
{
	if (!Accept(tokens))
	{
		AddError(Error::Code::SyntaxError, std::wstring(L"Unexpected character \"") + nextChar + L"\", expected \"" + join(tokens, ", ") + L" \".");
	}
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
			currentLexeme += currentChar;
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
			currentLexeme += currentChar;
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
		else if (std::iswdigit(nextChar) || !GetStringField(nextChar).empty())
		{
			state.stringFieldCode = nextChar;
			Accept();
		}
		else
		{
			AddError(Error::Code::StringFieldUnknown, std::wstring(L"Unknown string field code \"") + nextChar + L"\".");
		}
	}
	else if (Accept())
	{
		// Consume the string literal
		currentLexeme += currentChar;
	}
}

void Lexer::LexLiteralInteger(std::vector<Token>& tokens) noexcept
{
	if (Accept('.'))
	{
		state.integerLiteral = false;

		if (std::iswdigit(nextChar))
		{
			// Accept a single decimal point in numbers. Go from integer to decimal literal.
			currentLexeme += currentChar;
		}
		else
		{
			// The integer literal has ended, add it.
			tokens.emplace_back(GetCurrentLexemeToken());

			// Add the full stop token.
			tokens.emplace_back(GetCurrentCharToken());
		}
	}
	else if (std::iswdigit(nextChar))
	{
		Accept();
		currentLexeme += currentChar;
	}
	else
	{
		state.integerLiteral = false;

		// The integer literal has ended, add it.
		tokens.emplace_back(GetCurrentLexemeToken());
	}
}

void Lexer::LexParenAngleOpen(std::vector<Token>& tokens) noexcept
{
	if (currentChar == previousChar)
	{
		// Still opening comment.
	}
	else if (currentChar == nextChar)
	{
		// Begin opening comment.

		const auto commentLevel = state.commentLevel;
		if (!commentLevel)
		{
			// Add the token before the comment.
			tokens.emplace_back(GetCurrentLexemeToken());
		}

		state.commentLevel = commentLevel + 1;
	}
	else
	{
		// Not a comment separator, add the paren to the lexeme.
		currentLexeme += currentChar;
	}
}

void Lexer::LexParenAngleClose(std::vector<Token>&) noexcept
{
	if (currentChar == previousChar)
	{
		// Still closing comment.
	}
	else if (currentChar == nextChar)
	{
		// Begin closing comment.
		state.commentLevel--;

		currentLexeme.clear();
	}
	else
	{
		// Not a comment separator, add the paren to the lexeme.
		currentLexeme += currentChar;
	}
}

void Lexer::LexLiteralStringBegin(std::vector<Token>& tokens) noexcept
{
	// Add the token before the string literal.
	tokens.emplace_back(GetCurrentLexemeToken());

	state.stringLiteral = true;
}

void Lexer::LexSeparator(std::vector<Token>& tokens) noexcept
{
	// Add the token before the separator.
	tokens.emplace_back(GetCurrentLexemeToken());

	// Add the separator token.
	tokens.emplace_back(GetCurrentCharToken());
}

void Lexer::LexWhitespace(std::vector<Token>& tokens) noexcept
{
	// Add the token before the whitespace.
	tokens.emplace_back(GetCurrentLexemeToken());

	if (currentChar == '\n')
	{
		currentLine++;
		currentColumn = 1;
	}
}

void Lexer::LexLiteralIntegerBegin(std::vector<Token>&) noexcept
{
	currentLexeme += currentChar;

	state.integerLiteral = true;
}
