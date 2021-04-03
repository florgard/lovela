#include "pch.h"
#include "Lexer.h"

Lexer::Lexer(std::wistream& charStream) noexcept : charStream(charStream >> std::noskipws)
{
}

Token Lexer::AddToken(Token token) const
{
	token.line = currentLine;
	token.column = currentColumn;
	token.code = std::wstring(currentCode.begin(), currentCode.end());
	return token;
}

TokenGenerator Lexer::Lex() noexcept
{
	static constexpr std::wstring_view delimiters{ L"()[]{}.,:;!?|#" };

	std::wstring lexeme;
	currentLine = 1;
	currentColumn = 1;
	errors.clear();

	struct State
	{
		bool integerLiteral = false;
		bool stringLiteral = false;
		wchar_t stringFieldCode = 0;
		wchar_t nextStringInterpolation = '1';
		int commentLevel = 0;
	} state;

	charStream >> nextToken;

	while (nextToken)
	{
		if (state.stringFieldCode)
		{
			if (Accept('}'))
			{
				if (std::iswdigit(state.stringFieldCode))
				{
					// Indexed string interpolation. Add the string literal up to this point as a token.
					co_yield AddToken({ .type = Token::Type::LiteralString, .value = lexeme, .outType = stringTypeName });
					lexeme.clear();

					// Add a string literal interpolation token with the given index.
					co_yield AddToken({ .type = Token::Type::LiteralStringInterpolation, .value = std::wstring(1, state.stringFieldCode) });
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
			continue;
		}

		if (state.stringLiteral)
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
					co_yield AddToken({ .type = Token::Type::LiteralString, .value = lexeme, .outType = stringTypeName });
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
					co_yield AddToken({ .type = Token::Type::LiteralString, .value = lexeme, .outType = stringTypeName });
					lexeme.clear();

					// Add a string literal interpolation token with the next free index.
					if (state.nextStringInterpolation > '9')
					{
						AddError(Error::Code::StringInterpolationOverflow, std::wstring(L"Too many string interpolations, index out of bounds (greater than 9)."));
					}
					else
					{
						co_yield AddToken({ .type = Token::Type::LiteralStringInterpolation, .value = std::wstring(1, state.nextStringInterpolation) });
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

			continue;
		}

		if (state.integerLiteral)
		{
			if (Accept('.'))
			{
				if (std::iswdigit(nextToken))
				{
					// Accept a single decimal point in numbers. Go from integer to decimal literal.
					lexeme += currentToken;
					state.integerLiteral = false;
				}
				else
				{
					auto token = GetToken(lexeme);
					if (token)
					{
						co_yield AddToken(token);
					}
					lexeme.clear();
					state = State{};

					token = GetToken(currentToken);
					if (token)
					{
						co_yield AddToken(token);
					}
				}
				continue;
			}
			else if (std::iswdigit(nextToken))
			{
				Accept();
				lexeme += currentToken;
				continue;
			}
		}

		if (Accept('<'))
		{
			if (currentToken == previousToken)
			{
				// Still opening comment
			}
			else if (currentToken == nextToken)
			{
				// Begin opening comment
				state.commentLevel++;

				auto token = GetToken(lexeme);
				if (token)
				{
					co_yield AddToken(token);
				}
				lexeme.clear();
			}
			else
			{
				lexeme += currentToken;
			}

			continue;
		}

		if (Accept('>'))
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
			auto token = GetToken(lexeme);
			if (token)
			{
				co_yield AddToken(token);
			}
			lexeme.clear();
			state = State{};

			state.stringLiteral = true;
			continue;
		}
		else if (std::iswdigit(nextToken) && lexeme.empty())
		{
			Accept();
			lexeme += currentToken;
			state.integerLiteral = true;
			continue;
		}
		//else if (std::iswspace(nextToken))
		//{
		//	Accept();
		//	auto token = GetToken(lexeme);
		//	if (token)
		//	{
		//		co_yield AddToken(token);
		//	}
		//	lexeme.clear();
		//	state = State{};
		//	continue;
		//}
		else if (delimiters.find(nextToken) != delimiters.npos)
		{
			Accept();
			auto token = GetToken(lexeme);
			if (token)
			{
				co_yield AddToken(token);
			}
			lexeme.clear();
			state = State{};

			token = GetToken(currentToken);
			if (token)
			{
				co_yield AddToken(token);
			}
			continue;
		}

		if (std::iswspace(nextToken))
		{
			Accept();
			auto token = GetToken(lexeme);
			if (token)
			{
				co_yield AddToken(token);
			}
			lexeme.clear();
			state = State{};

			if (currentToken == '\n')
			{
				currentLine++;
				currentColumn = 1;
			}
		}
		else if (Accept())
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
		auto token = GetToken(lexeme);
		if (token)
		{
			co_yield AddToken(token);
		}
		lexeme.clear();

		co_yield AddToken({ .type = Token::Type::End });
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
