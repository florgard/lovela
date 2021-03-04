#include "pch.h"
#include "Lexer.h"

Lexer::Lexer(std::wistream& charStream) noexcept : charStream(charStream)
{
}

TokenGenerator Lexer::Lex() noexcept
{
	static constexpr std::wstring_view delimiters{ L"()[]{}.,:;!?|" };

	std::wstring lexeme;
	currentLine = 1;
	currentColumn = 1;
	errors.clear();

	struct State
	{
		bool skipNext = false;
		bool integerLiteral = false;
		bool stringLiteral = false;
		wchar_t stringFieldCode = 0;
		wchar_t nextStringInterpolation = '1';
		int commentLevel = 0;
	} state;

	wchar_t c = 0, prev = 0, next = 0;

	auto& stream = charStream >> std::noskipws;

	stream >> c;

	while (c)
	{
		next = 0;
		stream >> next;
	
		if (state.skipNext)
		{
			state.skipNext = false;
			goto readNext;
		}

		if (state.stringFieldCode)
		{
			if (c == '}')
			{
				if (std::iswdigit(state.stringFieldCode))
				{
					// Indexed string interpolation. Add the string literal up to this point as a token.
					co_yield{ .type = Token::Type::LiteralString, .value = lexeme };
					lexeme.clear();

					// Add a string literal interpolation token with the given index.
					co_yield{ .type = Token::Type::LiteralStringInterpolation, .value = std::wstring(1, state.stringFieldCode) };
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
			goto readNext;
		}

		if (state.stringLiteral)
		{
			if (c == '\'')
			{
				if (c == next)
				{
					// Keep a single escaped quotation mark
					lexeme += c;
					state.skipNext = true;
				}
				else
				{
					co_yield{ .type = Token::Type::LiteralString, .value = lexeme };
					lexeme.clear();

					state.stringLiteral = false;
					state.nextStringInterpolation = '1';
				}
			}
			else if (c == '{')
			{
				if (c == next)
				{
					// Keep a single escaped curly bracket
					lexeme += c;
					state.skipNext = true;
				}
				else if (next == '}')
				{
					// Unindexed string interpolation. Add the string literal up to this point as a token.
					co_yield{ .type = Token::Type::LiteralString, .value = lexeme };
					lexeme.clear();

					// Add a string literal interpolation token with the next free index.
					if (state.nextStringInterpolation > '9')
					{
						AddError(Error::Code::StringInterpolationOverflow, std::wstring(L"Too many string interpolations, index out of bounds (greater than 9)."));
					}
					else
					{
						co_yield{ .type = Token::Type::LiteralStringInterpolation, .value = std::wstring(1, state.nextStringInterpolation) };
						state.nextStringInterpolation++;
					}

					state.skipNext = true;
				}
				else if (std::iswdigit(next) || !GetStringField(next).empty())
				{
					state.stringFieldCode = next;
					state.skipNext = true;
				}
				else
				{
					AddError(Error::Code::StringFieldUnknown, std::wstring(L"Unknown string field code \"") + next + L"\".");
				}
			}
			else
			{
				// Consume the string literal
				lexeme += c;
			}

			goto readNext;
		}

		if (state.integerLiteral && c == '.' && std::iswdigit(next))
		{
			// Accept a single decimal point in numbers. Go from integer to decimal literal.
			state.integerLiteral = false;
			lexeme += c;
			goto readNext;
		}

		if (c == '<')
		{
			if (c == prev)
			{
				// Still opening comment
				goto readNext;
			}
			else if (c == next)
			{
				// Begin opening comment
				state.commentLevel++;

				auto token = GetToken(lexeme);
				if (!token.empty())
				{
					co_yield token;
				}
				lexeme.clear();
				goto readNext;
			}
		}

		if (c == '>')
		{
			if (c == prev)
			{
				// Still closing comment
				goto readNext;
			}
			else if (c == next)
			{
				// Begin closing comment
				state.commentLevel--;

				lexeme.clear();
				goto readNext;
			}
		}

		if (state.commentLevel)
		{
			// Consume the comment
			goto readNext;
		}

		if (c == '\'')
		{
			auto token = GetToken(lexeme);
			if (!token.empty())
			{
				co_yield token;
			}
			lexeme.clear();
			state = State{};

			state.stringLiteral = true;
			goto readNext;
		}
		else if (std::iswdigit(c) && lexeme.empty())
		{
			state.integerLiteral = true;
		}
		else if (std::iswspace(c))
		{
			auto token = GetToken(lexeme);
			if (!token.empty())
			{
				co_yield token;
			}
			lexeme.clear();
			state = State{};
		}
		else if (delimiters.find(c) != delimiters.npos)
		{
			auto token = GetToken(lexeme);
			if (!token.empty())
			{
				co_yield token;
			}
			lexeme.clear();
			state = State{};

			token = GetToken(c);
			if (!token.empty())
			{
				co_yield token;
			}
			goto readNext;
		}

		if (!std::iswspace(c))
		{
			lexeme += c;
		}
		else if (c == '\n')
		{
			currentLine++;
			currentColumn = 1;
		}

	readNext:
		prev = c;
		c = next;
		currentColumn++;
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
		if (!token.empty())
		{
			co_yield token;
		}
		lexeme.clear();
	}
}
