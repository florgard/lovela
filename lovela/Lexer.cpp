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
			auto t = YieldToken(WordBreak());
			if (t) co_yield t;
		}
		else if (Accept(patterns.separator))
		{
			auto t = YieldToken(WordBreak());
			if (t) co_yield t;

			t = YieldToken(LexSeparator());
			if (t) co_yield t;
		}
		else if (Accept(patterns.beginComment))
		{
			auto t = YieldToken(WordBreak());
			if (t) co_yield t;

			for (auto&& rt : LexComment())
			{
				t = YieldToken(rt);
				if (t) co_yield t;
			}
		}
		else if (IsWordBreakExpected())
		{
			auto t = YieldToken(WordBreak());
			if (t) co_yield t;

			t = YieldToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::SyntaxError, .message = fmt::format("Unexpected character \"{}\".", GetCharacter(Next)) } });
			if (t) co_yield t;
		}
		else if (AcceptBegin(patterns.beginLiteralNumber))
		{
			auto t = YieldToken(LexLiteralNumber());
			if (t) co_yield t;

			ExpectWordBreak();
		}
		else if (AcceptBegin(patterns.beginString))
		{
			for (auto&& rt : LexLiteralString())
			{
				auto t = YieldToken(rt);
				if (t) co_yield t;
			}

			ExpectWordBreak();
		}
		else if (Accept())
		{
			// Append to the current lexeme.
			AddCharacter(GetCharacter(Current));
		}
	}

	// Add the possible token at the very end of the stream.
	auto t = YieldToken(WordBreak());
	if (t) co_yield t;

	// Add the end token.
	t = YieldToken({.type = Token::Type::End});
	if (t) co_yield t;

	// Add the last code line.
	AddCodeLine();
}

ILexer::OutputT Lexer::LexLiteralString() noexcept
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
				Token t{.type = Token::Type::LiteralString, .value = MoveClear(value)};
				co_yield t;
				co_return;
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
				Token t{.type = Token::Type::LiteralString, .value = MoveClear(value)};
				co_yield t;

				// Add a string literal interpolation token with the next free index.
				if (nextStringInterpolation > '9')
				{
					t = { .type = Token::Type::Error, .error{.code = Token::Error::Code::StringInterpolationOverflow, .message = "Too many string interpolations, index out of bounds (greater than 9)." } };
					co_yield t;
				}
				else
				{
					t = {.type = Token::Type::LiteralStringInterpolation, .value = std::string(1, nextStringInterpolation)};
					co_yield t;
					nextStringInterpolation++;
				}
			}
			else if (Accept(patterns.digit))
			{
				char stringFieldCode = GetCharacter(Current);

				if (Accept(patterns.stringFieldClose))
				{
					// Indexed string interpolation. Add the string literal up to this point as a token.
					Token t{.type = Token::Type::LiteralString, .value = MoveClear(value)};
					co_yield t;

					// Add a string literal interpolation token with the given index.
					t = {.type = Token::Type::LiteralStringInterpolation, .value = std::string(1, stringFieldCode)};
					co_yield t;
				}
				else
				{
					Token t{ .type = Token::Type::Error, .error{.code = Token::Error::Code::StringFieldIllformed, .message = fmt::format("Ill-formed string field \"{}\".", stringFieldCode), } };
					co_yield t;
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
					Token t{ .type = Token::Type::Error, .error{.code = Token::Error::Code::StringFieldIllformed, .message = fmt::format("Ill-formed string field \"{}\".", stringFieldCode) } };
					co_yield t;
				}
			}
			else
			{
				Token t{ .type = Token::Type::Error, .error{.code = Token::Error::Code::StringFieldUnknown, .message = fmt::format("Unknown string field code \"{}\".", GetCharacter(Next)) } };
				co_yield t;
			}
		}
		else if (Accept())
		{
			// Consume the string literal
			value += GetCharacter(Current);
		}
		else
		{
			Token t{ .type = Token::Type::Error, .error{.code = Token::Error::Code::StringLiteralOpen, .message = "A string literal wasn't terminated." } };
			co_yield t;
			co_return;
		}
	}
}

Token Lexer::LexLiteralNumber() noexcept
{
	// FIXME: Optimize with a simple state machine.

	std::string value;
	value += GetCharacter(Current);

	while (Accept(patterns.digit))
		value += GetCharacter(Current);

	// Accept a single decimal point in numbers.

	if (Accept(patterns.beginDecimalPart))
	{
		value += GetCharacter(Current);

		while (Accept(patterns.digit))
			value += GetCharacter(Current);

		if (Accept(patterns.beginDecimalExponent))
		{
			value += GetCharacter(Current);

			if (!Accept(patterns.beginLiteralNumber))
				return { .type = Token::Type::Error, .error{.code = Token::Error::Code::LiteralDecimalIllformed, .message = "Ill-formed literal decimal number." } };
			
			value += GetCharacter(Current);

			while (Accept(patterns.digit))
				value += GetCharacter(Current);
		}

		return {.type = Token::Type::LiteralDecimal, .value = MoveClear(value)};
	}
	else
	{
		return {.type = Token::Type::LiteralInteger, .value = MoveClear(value)};
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

			for (Token t : LexComment())
				co_yield t;
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

Token Lexer::LexSeparator() noexcept
{
	return GetToken(GetCharacter(Current));
}

Token Lexer::GetToken(char lexeme) noexcept
{
	auto type = GetTokenType(lexeme);
	if (type == Token::Type::Empty)
		return {};

	return { .type = type, .value = std::string(1, lexeme) };
}

Token Lexer::GetToken(std::string_view lexeme) noexcept
{
	const auto trimmed = Trim(lexeme);

	if (trimmed.empty())
		return {};

	// Check for single char token

	if (trimmed.length() == 1)
	{
		Token t = GetToken(trimmed[0]);
		if (t) return t;
	}

	// Check for ANSI token

	for (const auto& lexemePattern : patterns.lexemePatternsAnsi)
		if (std::regex_match(trimmed.begin(), trimmed.end(), lexemePattern.pattern.regex))
			return { .type = lexemePattern.tokenType, .value = std::string(trimmed.data(), trimmed.size()) };

	// Check for Unicode token

	const auto wlexeme = to_wstring(lexeme);
	const auto wtrimmed = Trim(wlexeme);

	for (const auto& lexemePattern : patterns.lexemePatternsUnicode)
		if (std::regex_match(wtrimmed.begin(), wtrimmed.end(), lexemePattern.pattern.regex))
			return { .type = lexemePattern.tokenType, .value = std::string(trimmed.data(), trimmed.size()) };

	return { .type = Token::Type::Error, .error{.code = Token::Error::Code::SyntaxError, .message = fmt::format("Syntax error near \"{}\".", std::string(trimmed.data(), trimmed.size()))} };
}
