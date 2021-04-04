#include "pch.h"
#include "Lexer.h"

Lexer::Lexer(std::wistream& charStream) noexcept : charStream(charStream >> std::noskipws)
{
}

Token Lexer::GetCurrentLexemeToken()
{
	auto token = DecorateToken(GetToken(currentLexeme));
	currentLexeme.clear();
	state.Clear();
	return token;
}

Token Lexer::GetCurrentCharToken()
{
	auto token = DecorateToken(GetToken(currentChar));
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
	static constexpr std::wstring_view separators{ L"()[]{}.,:;!?|#" };

	currentLexeme.clear();
	currentLine = 1;
	currentColumn = 1;
	errors.clear();
	state.Clear();

	charStream >> nextChar;

	while (nextChar)
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

		if (std::iswdigit(nextChar) && currentLexeme.empty())
		{
			Accept();
			for (auto token : LexLiteralIntegerBegin())
			{
				co_yield token;
			}
			continue;
		}

		if (separators.find(nextChar) != separators.npos)
		{
			Accept();
			for (auto token : LexSeparator())
			{
				co_yield token;
			}
			continue;
		}

		if (std::iswspace(nextChar))
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
			currentLexeme += currentChar;
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

TokenGenerator Lexer::LexStringFieldCode() noexcept
{
	if (Accept('}'))
	{
		if (std::iswdigit(state.stringFieldCode))
		{
			// Indexed string interpolation. Add the string literal up to this point as a token.
			co_yield { .type = Token::Type::LiteralString, .value = currentLexeme, .outType = stringTypeName };
			currentLexeme.clear();

			// Add a string literal interpolation token with the given index.
			co_yield { .type = Token::Type::LiteralStringInterpolation, .value = std::wstring(1, state.stringFieldCode) };
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

TokenGenerator Lexer::LexLiteralString() noexcept
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
			co_yield { .type = Token::Type::LiteralString, .value = currentLexeme, .outType = stringTypeName };
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
			co_yield { .type = Token::Type::LiteralString, .value = currentLexeme, .outType = stringTypeName };
			currentLexeme.clear();

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

TokenGenerator Lexer::LexLiteralInteger() noexcept
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
			auto token = GetCurrentLexemeToken();
			if (token)
			{
				co_yield token;
			}

			token = GetCurrentCharToken();
			if (token)
			{
				co_yield token;
			}
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

		auto token = GetCurrentLexemeToken();
		if (token)
		{
			co_yield token;
		}
	}
}

TokenGenerator Lexer::LexParenAngleOpen() noexcept
{
	if (currentChar == previousChar)
	{
		// Still opening comment
	}
	else if (currentChar == nextChar)
	{
		// Begin opening comment
		const auto commentLevel = state.commentLevel + 1;

		auto token = GetCurrentLexemeToken();
		if (token)
		{
			co_yield token;
		}

		state.commentLevel = commentLevel;
	}
	else
	{
		currentLexeme += currentChar;
	}
}

TokenGenerator Lexer::LexParenAngleClose() noexcept
{
	if (currentChar == previousChar)
	{
		// Still closing comment
	}
	else if (currentChar == nextChar)
	{
		// Begin closing comment
		state.commentLevel--;

		currentLexeme.clear();
	}
	else
	{
		currentLexeme += currentChar;
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

	state.stringLiteral = true;
}

TokenGenerator Lexer::LexSeparator() noexcept
{
	auto token = GetCurrentLexemeToken();
	if (token)
	{
		co_yield token;
	}

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

	if (currentChar == '\n')
	{
		currentLine++;
		currentColumn = 1;
	}
}

TokenGenerator Lexer::LexLiteralIntegerBegin() noexcept
{
	currentLexeme += currentChar;
	state.integerLiteral = true;

	// HACK to avoid a compiler error for a coroutine without co_yield.
	if (false)
	{
		co_yield {};
	}
}
