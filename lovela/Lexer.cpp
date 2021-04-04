#include "pch.h"
#include "Lexer.h"

Lexer::Lexer(std::wistream& charStream) noexcept : charStream(charStream >> std::noskipws)
{
}

Token Lexer::GetCurrentLexemeToken()
{
	auto token = DecorateToken(GetToken(lexeme));
	lexeme.clear();
	return token;
}

Token Lexer::GetCurrentCharToken()
{
	auto token = DecorateToken(GetToken(currentToken));
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
	static constexpr std::wstring_view delimiters{ L"()[]{}.,:;!?|#" };

	lexeme.clear();
	currentLine = 1;
	currentColumn = 1;
	errors.clear();
	state.Clear();

	charStream >> nextToken;

	while (nextToken)
	{
		if (state.stringFieldCode)
		{
			for (auto token : LexStringFieldCode())
			{
				co_yield token;
			}
			continue;
		}

		if (state.stringLiteral)
		{
			for (auto token : LexLiteralString())
			{
				co_yield token;
			}
			continue;
		}

		if (state.integerLiteral)
		{
			for (auto token : LexLiteralInteger())
			{
				co_yield token;
			}
			continue;
		}

		if (Accept('<'))
		{
			for (auto token : LexParenAngleOpen())
			{
				co_yield token;
			}
			continue;
		}

		if (Accept('>'))
		{
			for (auto token : LexParenAngleClose())
			{
				co_yield token;
			}
			continue;
		}

		if (state.commentLevel)
		{
			// Consume the comment
			Accept();
			continue;
		}

		if (Accept('\''))
		{
			for (auto token : LexLiteralStringBegin())
			{
				co_yield token;
			}
			continue;
		}
		else if (std::iswdigit(nextToken) && lexeme.empty())
		{
			Accept();
			for (auto token : LexLiteralIntegerBegin())
			{
				co_yield token;
			}
			continue;
		}
		else if (delimiters.find(nextToken) != delimiters.npos)
		{
			Accept();
			for (auto token : LexSeparator())
			{
				co_yield token;
			}
			continue;
		}

		if (std::iswspace(nextToken))
		{
			Accept();
			for (auto token : LexWhitespace())
			{
				co_yield token;
			}
			continue;
		}

		if (Accept())
		{
			lexeme += currentToken;
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
			co_yield token;
		}

		co_yield DecorateToken({ .type = Token::Type::End });
	}
}

bool Lexer::Accept()
{
	currentColumn++;

	if (nextToken)
	{
		previousToken = currentToken;
		currentToken = nextToken;
		nextToken = 0;
		charStream >> nextToken;

		static constexpr size_t codeSampleCharacters = 20;
		currentCode.push_back(currentToken);
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
	return token == nextToken;
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
	if (std::find(tokens.begin(), tokens.end(), nextToken) != tokens.end())
	{
		return Accept();
	}

	return false;
}

void Lexer::Expect(wchar_t token)
{
	if (!Accept(token))
	{
		AddError(Error::Code::SyntaxError, std::wstring(L"Unexpected character \"") + nextToken + L"\", expected \"" + token + L" \".");
	}
}

void Lexer::Expect(const std::vector<wchar_t>& tokens)
{
	if (!Accept(tokens))
	{
		AddError(Error::Code::SyntaxError, std::wstring(L"Unexpected character \"") + nextToken + L"\", expected \"" + join(tokens, ", ") + L" \".");
	}
}

TokenGenerator Lexer::LexStringFieldCode() noexcept
{
	if (Accept('}'))
	{
		if (std::iswdigit(state.stringFieldCode))
		{
			// Indexed string interpolation. Add the string literal up to this point as a token.
			co_yield { .type = Token::Type::LiteralString, .value = lexeme, .outType = stringTypeName };
			lexeme.clear();

			// Add a string literal interpolation token with the given index.
			co_yield { .type = Token::Type::LiteralStringInterpolation, .value = std::wstring(1, state.stringFieldCode) };
		}
		else
		{
			// Add the string field value to the string literal.
			lexeme += GetStringField(state.stringFieldCode);
		}
	}
	else
	{
		AddError(Error::Code::StringFieldIllformed, std::wstring(L"Ill-formed string field \"") + state.stringFieldCode + L"\".");
	}

	state.stringFieldCode = 0;
}

TokenGenerator Lexer::LexLiteralString() noexcept
{
	if (Accept('\''))
	{
		if (Accept('\''))
		{
			// Keep a single escaped quotation mark
			lexeme += currentToken;
		}
		else
		{
			co_yield { .type = Token::Type::LiteralString, .value = lexeme, .outType = stringTypeName };
			lexeme.clear();

			state.stringLiteral = false;
			state.nextStringInterpolation = '1';
		}
	}
	else if (Accept('{'))
	{
		if (Accept('{'))
		{
			// Keep a single escaped curly bracket
			lexeme += currentToken;
		}
		else if (Accept('}'))
		{
			// Unindexed string interpolation. Add the string literal up to this point as a token.
			co_yield { .type = Token::Type::LiteralString, .value = lexeme, .outType = stringTypeName };
			lexeme.clear();

			// Add a string literal interpolation token with the next free index.
			if (state.nextStringInterpolation > '9')
			{
				AddError(Error::Code::StringInterpolationOverflow, std::wstring(L"Too many string interpolations, index out of bounds (greater than 9)."));
			}
			else
			{
				co_yield { .type = Token::Type::LiteralStringInterpolation, .value = std::wstring(1, state.nextStringInterpolation) };
				state.nextStringInterpolation++;
			}
		}
		else if (std::iswdigit(nextToken) || !GetStringField(nextToken).empty())
		{
			state.stringFieldCode = nextToken;
			Accept();
		}
		else
		{
			AddError(Error::Code::StringFieldUnknown, std::wstring(L"Unknown string field code \"") + nextToken + L"\".");
		}
	}
	else if (Accept())
	{
		// Consume the string literal
		lexeme += currentToken;
	}
}

TokenGenerator Lexer::LexLiteralInteger() noexcept
{
	if (Accept('.'))
	{
		state.integerLiteral = false;

		if (std::iswdigit(nextToken))
		{
			// Accept a single decimal point in numbers. Go from integer to decimal literal.
			lexeme += currentToken;
		}
		else
		{
			auto token = GetCurrentLexemeToken();
			if (token)
			{
				co_yield token;
			}
			state = State{};

			token = GetCurrentCharToken();
			if (token)
			{
				co_yield token;
			}
		}
	}
	else if (std::iswdigit(nextToken))
	{
		Accept();
		lexeme += currentToken;
	}
	else
	{
		state.integerLiteral = false;

		auto token = GetCurrentLexemeToken();
		if (token)
		{
			co_yield token;
		}
		state = State{};
	}
}

TokenGenerator Lexer::LexParenAngleOpen() noexcept
{
	if (currentToken == previousToken)
	{
		// Still opening comment
	}
	else if (currentToken == nextToken)
	{
		// Begin opening comment
		state.commentLevel++;

		auto token = GetCurrentLexemeToken();
		if (token)
		{
			co_yield token;
		}
	}
	else
	{
		lexeme += currentToken;
	}
}

TokenGenerator Lexer::LexParenAngleClose() noexcept
{
	if (currentToken == previousToken)
	{
		// Still closing comment
	}
	else if (currentToken == nextToken)
	{
		// Begin closing comment
		state.commentLevel--;

		lexeme.clear();
	}
	else
	{
		lexeme += currentToken;
	}

	// HACK to avoid a compiler error for a coroutine without co_yield.
	if (false)
	{
		co_yield {};
	}
}

TokenGenerator Lexer::LexLiteralStringBegin() noexcept
{
	auto token = GetCurrentLexemeToken();
	if (token)
	{
		co_yield token;
	}
	state = State{};

	state.stringLiteral = true;
}

TokenGenerator Lexer::LexSeparator() noexcept
{
	auto token = GetCurrentLexemeToken();
	if (token)
	{
		co_yield token;
	}
	state = State{};

	token = GetCurrentCharToken();
	if (token)
	{
		co_yield token;
	}
}

TokenGenerator Lexer::LexWhitespace() noexcept
{
	auto token = GetCurrentLexemeToken();
	if (token)
	{
		co_yield token;
	}
	state = State{};

	if (currentToken == '\n')
	{
		currentLine++;
		currentColumn = 1;
	}
}

TokenGenerator Lexer::LexLiteralIntegerBegin() noexcept
{
	lexeme += currentToken;
	state.integerLiteral = true;

	// HACK to avoid a compiler error for a coroutine without co_yield.
	if (false)
	{
		co_yield {};
	}
}
