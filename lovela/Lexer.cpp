#include "pch.h"
#include "Lexer.h"

Lexer::Lexer(std::istream& charStream) noexcept
	: LexerBase(charStream)
{
}

TokenGenerator Lexer::Lex() noexcept
{
	// Populate next and next after characters.
	GetNextCharacter();
	GetNextCharacter();

	while (GetCharacter(Next))
	{
		if (Accept(regexes.GetWhitespace(), 1))
		{
			WordBreak();
			LexWhitespace();
		}
		else if (Accept(regexes.GetSeparator(), 1))
		{
			WordBreak();
			LexSeparator();
		}
		else if (Accept(regexes.GetBeginComment(), 2))
		{
			WordBreak();
			LexComment();
		}
		else if (IsWordBreakExpected())
		{
			WordBreak();
			AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::SyntaxError, .message = std::format("Unexpected character \"{}\".", GetCharacter(Next)) } });
		}
		else if (AcceptBegin(regexes.GetBeginLiteralNumber(), 2))
		{
			LexLiteralNumber();
			ExpectWordBreak();
		}
		else if (AcceptBegin(regexes.GetBeginString(), 1))
		{
			LexLiteralString();
			ExpectWordBreak();
		}
		else if (Accept())
		{
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
		if (Accept('\''))
		{
			if (Accept('\''))
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
		else if (Accept('{'))
		{
			if (Accept('{'))
			{
				// Keep a single escaped curly bracket
				value += GetCharacter(Current);
			}
			else if (Accept('}'))
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
			else if (Accept(regexes.GetDigit(), 1))
			{
				char stringFieldCode = GetCharacter(Current);

				if (Accept('}'))
				{
					// Indexed string interpolation. Add the string literal up to this point as a token.
					AddToken({.type = Token::Type::LiteralString, .value = std::move(value)});

					// Add a string literal interpolation token with the given index.
					AddToken({.type = Token::Type::LiteralStringInterpolation, .value = std::string(1, stringFieldCode)});
				}
				else
				{
					AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::StringFieldIllformed, .message = std::format("Ill-formed string field \"{}\".", stringFieldCode), } });
				}
			}
			else if (Accept(regexes.GetStringField(), 1))
			{
				char stringFieldCode = GetCharacter(Current);

				if (Accept('}'))
				{
					// Add the string field value to the string literal.
					value += GetStringField(stringFieldCode);
				}
				else
				{
					AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::StringFieldIllformed, .message = std::format("Ill-formed string field \"{}\".", stringFieldCode) } });
				}
			}
			else
			{
				AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::StringFieldUnknown, .message = std::format("Unknown string field code \"{}\".", GetCharacter(Next)) } });
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

	auto& digitRegex = regexes.GetDigit();
	while (Accept(digitRegex, 1))
	{
		value += GetCharacter(Current);
	}

	// Accept a single decimal point in numbers.

	if (Accept(regexes.GetBeginDecimalPart(), 2))
	{
		value += GetCharacter(Current);

		while (Accept(digitRegex, 1))
		{
			value += GetCharacter(Current);
		}

		if (Accept(regexes.GetBeginDecimalExponent(), 2))
		{
			value += GetCharacter(Current);

			if (!Accept(regexes.GetBeginLiteralNumber(), 2))
			{
				AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::LiteralDecimalIllformed, .message = "Ill-formed literal decimal number." } });
				return;
			}
			
			value += GetCharacter(Current);

			while (Accept(digitRegex, 1))
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

void Lexer::LexComment() noexcept
{
	while (Accept('<'));

	int commentLevel = 1;

	for (;;)
	{
		if (Accept(regexes.GetEndComment(), 2))
		{
			while (Accept('>'));

			commentLevel--;

			if (!commentLevel)
			{
				return;
			}
		}
		else if (Accept(regexes.GetBeginComment(), 2))
		{
			// Nested comment.

			while (Accept('<'));

			commentLevel++;
		}
		else if (Accept())
		{
			// Consume the comment.
		}
		else
		{
			AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::CommentOpen, .message = "A comment wasn't terminated." } });
			return;
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
	// https://en.cppreference.com/w/cpp/regex/ecmascript
	// https://www.regular-expressions.info/posixbrackets.html
	// https://www.regular-expressions.info/unicode.html
	// https://en.wikipedia.org/wiki/Combining_character

	// ANSI token regexes
	static const std::vector<std::pair<std::regex, Token::Type>> definitions
	{
		{ std::regex{ R"(<|>|<>|<=|>=|=)" }, Token::Type::OperatorComparison },
		{ std::regex{ R"(\+|-|\*|/|/*)" }, Token::Type::OperatorArithmetic },
		{ std::regex{ R"(\*\*|\+\+|--)" }, Token::Type::OperatorBitwise },
		{ std::regex{ R"(<-|->)" }, Token::Type::OperatorArrow },
		{ std::regex{ R"([[:alpha:]][\w<>=\+\-\*/]*)" }, Token::Type::Identifier },
	};

	// Additional Unicode token regexes
	static const std::vector<std::pair<std::wregex, Token::Type>> unicodeDefinitions
	{
		{ std::wregex{ LR"([[:alpha:]][\w<>=\+\-\*/\u0300–\u036F\u1AB0–\u1AFF\u1DC0–\u1DFF\u20D0–\u20FF\uFE20–\uFE2F]*)" }, Token::Type::Identifier },
	};

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

	for (const auto& definition : definitions)
	{
		if (std::regex_match(trimmed.begin(), trimmed.end(), definition.first))
		{
			return { .type = definition.second, .value = std::string(trimmed.data(), trimmed.size()) };
		}
	}

	// Check for Unicode token

	const auto wlexeme = to_wstring(lexeme);
	const auto wtrimmed = Trim(wlexeme);

	for (const auto& unicodeDefinition : unicodeDefinitions)
	{
		if (std::regex_match(wtrimmed.begin(), wtrimmed.end(), unicodeDefinition.first))
		{
			return { .type = unicodeDefinition.second, .value = std::string(trimmed.data(), trimmed.size()) };
		}
	}

	AddToken({ .type = Token::Type::Error, .error{.code = Token::Error::Code::SyntaxError, .message = std::format("Syntax error near \"{}\".", std::string(trimmed.data(), trimmed.size()))} });

	return {};
}
