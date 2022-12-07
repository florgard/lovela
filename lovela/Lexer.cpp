#include "pch.h"
#include "Lexer.h"

ILexer::OutputT Lexer::Lex() noexcept
{
	// Populate next and next after characters.
	GetNextCharacter();
	GetNextCharacter();

	while (GetCharacter(Next))
	{
		if (Accept(patterns.whitespace))
		{
			WordBreak();
			LexWhitespace();
		}
		else if (Accept(patterns.separator))
		{
			WordBreak();
			LexSeparator();
		}
		else if (Accept(patterns.beginComment))
		{
			WordBreak();
			for (auto t : LexComment())
			{
				AddToken(std::move(t));
			}
		}
		else if (IsWordBreakExpected())
		{
			WordBreak();
			AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::SyntaxError, .message = fmt::format("Unexpected character \"{}\".", GetCharacter(Next)) } });
		}
		else if (AcceptBegin(patterns.beginLiteralNumber))
		{
			LexLiteralNumber();
			ExpectWordBreak();
		}
		else if (AcceptBegin(patterns.beginString))
		{
			LexLiteralString();
			ExpectWordBreak();
		}
		else if (Accept())
		{
			// Append to the current lexeme.
			AddCharacter(GetCharacter(Current));
		}

		for (auto& token : GetCurrentTokens())
		{
			co_yield token;
		}

		ClearCurrentTokens();
	}

	// Add the possible token at the very end of the stream.
	WordBreak();

	// Add the end token.
	AddToken({.type = Token::Type::End});

	// Add the last code line.
	AddCodeLine();

	for (auto& token : GetCurrentTokens())
	{
		co_yield token;
	}
}

void Lexer::LexLiteralString() noexcept
{
	std::string value;
	char nextStringInterpolation = '1';

	for (;;)
	{
		if (Accept(patterns.stringOpen))
		{
			if (Accept(patterns.stringOpen))
			{
				// Keep a single escaped quotation mark
				value += GetCharacter(Current);
			}
			else
			{
				AddToken({.type = Token::Type::LiteralString, .value = std::move(value)});
				return;
			}
		}
		else if (Accept(patterns.stringFieldOpen))
		{
			if (Accept(patterns.stringFieldOpen))
			{
				// Keep a single escaped curly bracket
				value += GetCharacter(Current);
			}
			else if (Accept(patterns.stringFieldClose))
			{
				// Unindexed string interpolation. Add the string literal up to this point as a token.
				AddToken({.type = Token::Type::LiteralString, .value = std::move(value)});

				// Add a string literal interpolation token with the next free index.
				if (nextStringInterpolation > '9')
				{
					AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::StringInterpolationOverflow, .message = "Too many string interpolations, index out of bounds (greater than 9)." } });
				}
				else
				{
					AddToken({.type = Token::Type::LiteralStringInterpolation, .value = std::string(1, nextStringInterpolation)});
					nextStringInterpolation++;
				}
			}
			else if (Accept(patterns.digit))
			{
				char stringFieldCode = GetCharacter(Current);

				if (Accept(patterns.stringFieldClose))
				{
					// Indexed string interpolation. Add the string literal up to this point as a token.
					AddToken({.type = Token::Type::LiteralString, .value = std::move(value)});

					// Add a string literal interpolation token with the given index.
					AddToken({.type = Token::Type::LiteralStringInterpolation, .value = std::string(1, stringFieldCode)});
				}
				else
				{
					AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::StringFieldIllformed, .message = fmt::format("Ill-formed string field \"{}\".", stringFieldCode), } });
				}
			}
			else if (Accept(patterns.stringField))
			{
				char stringFieldCode = GetCharacter(Current);

				if (Accept(patterns.stringFieldClose))
				{
					// Add the string field value to the string literal.
					value += GetStringField(stringFieldCode);
				}
				else
				{
					AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::StringFieldIllformed, .message = fmt::format("Ill-formed string field \"{}\".", stringFieldCode) } });
				}
			}
			else
			{
				AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::StringFieldUnknown, .message = fmt::format("Unknown string field code \"{}\".", GetCharacter(Next)) } });
			}
		}
		else if (Accept())
		{
			// Consume the string literal
			value += GetCharacter(Current);
		}
		else
		{
			AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::StringLiteralOpen, .message = "A string literal wasn't terminated." } });
			return;
		}
	}
}

void Lexer::LexLiteralNumber() noexcept
{
	// FIXME: Optimize with a simple state machine.

	std::string value;
	value += GetCharacter(Current);

	while (Accept(patterns.digit))
	{
		value += GetCharacter(Current);
	}

	// Accept a single decimal point in numbers.

	if (Accept(patterns.beginDecimalPart))
	{
		value += GetCharacter(Current);

		while (Accept(patterns.digit))
		{
			value += GetCharacter(Current);
		}

		if (Accept(patterns.beginDecimalExponent))
		{
			value += GetCharacter(Current);

			if (!Accept(patterns.beginLiteralNumber))
			{
				AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::LiteralDecimalIllformed, .message = "Ill-formed literal decimal number." } });
				return;
			}
			
			value += GetCharacter(Current);

			while (Accept(patterns.digit))
			{
				value += GetCharacter(Current);
			}
		}

		AddToken({.type = Token::Type::LiteralDecimal, .value = std::move(value)});
	}
	else
	{
		AddToken({.type = Token::Type::LiteralInteger, .value = std::move(value)});
	}
}

ILexer::OutputT Lexer::LexComment() noexcept
{
	// Consume consecutive angle brackets.
	while (Accept(patterns.commentOpen));

	Token token{ .type = Token::Type::Comment };

	for (;;)
	{
		if (Accept(patterns.endComment))
		{
			// Consume consecutive angle brackets.
			while (Accept(patterns.commentClose));

			co_yield token;

			co_return;
		}
		else if (Accept(patterns.beginComment))
		{
			// Nested comment.

			co_yield token;
			token = { .type = Token::Type::Comment };

			for (auto t : LexComment())
			{
				co_yield t;
			}
		}
		else if (Accept())
		{
			// Consume the comment.
			token.value += GetCharacter(Current);
		}
		else
		{
			co_yield token;

			token = { .type = Token::Type::Error, .error{.code = Token::Error::Code::CommentOpen, .message = "A comment wasn't terminated." } };
			co_yield token;

			co_return;
		}
	}
}

void Lexer::LexSeparator() noexcept
{
	AddToken(GetToken(GetCharacter(Current)));
}

void Lexer::LexWhitespace() noexcept
{
}

Token Lexer::GetToken(char lexeme) noexcept
{
	auto type = GetTokenType(lexeme);
	if (type == Token::Type::Empty)
	{
		return {};
	}

	return { .type = type, .value = std::string(1, lexeme) };
}

Token Lexer::GetToken(std::string_view lexeme) noexcept
{
	const auto trimmed = Trim(lexeme);

	if (trimmed.empty())
	{
		return {};
	}

	// Check for single char token

	if (trimmed.length() == 1)
	{
		auto token = GetToken(trimmed[0]);
		if (token)
		{
			return token;
		}
	}

	// Check for ANSI token

	for (const auto& lexemePattern : patterns.lexemePatternsAnsi)
	{
		if (std::regex_match(trimmed.begin(), trimmed.end(), lexemePattern.pattern.regex))
		{
			return { .type = lexemePattern.tokenType, .value = std::string(trimmed.data(), trimmed.size()) };
		}
	}

	// Check for Unicode token

	const auto wlexeme = to_wstring(lexeme);
	const auto wtrimmed = Trim(wlexeme);

	for (const auto& lexemePattern : patterns.lexemePatternsUnicode)
	{
		if (std::regex_match(wtrimmed.begin(), wtrimmed.end(), lexemePattern.pattern.regex))
		{
			return { .type = lexemePattern.tokenType, .value = std::string(trimmed.data(), trimmed.size()) };
		}
	}

	return { .type = Token::Type::Error, .error{.code = Token::Error::Code::SyntaxError, .message = fmt::format("Syntax error near \"{}\".", std::string(trimmed.data(), trimmed.size()))} };
}
