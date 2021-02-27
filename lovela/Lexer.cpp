#include "Lexer.h"
#include <string>
#include <string_view>
#include <regex>
#include <algorithm>
#include <iostream>
#include <cwctype>

using namespace std::literals::string_view_literals;

static_assert(LexerBase::Trim(L""sv) == L""sv);
static_assert(LexerBase::Trim(L" \t\r\n\f\v"sv) == L""sv);
static_assert(LexerBase::Trim(L"a b c \r\n"sv) == L"a b c"sv);

static_assert(LexerBase::GetTokenType('(') == TokenType::ParenRoundOpen);
static_assert(LexerBase::GetTokenType('.') == TokenType::SeparatorDot);
static_assert(LexerBase::GetTokenType(' ') == TokenType::Unknown);

void Lexer::AddToken(const std::wstring_view& lexeme, std::vector<Token>& tokens)
{
	static const std::vector<std::pair<std::wregex, TokenType>> tokenRegexes{
		{ std::wregex{ LR"(\d+)" }, TokenType::LiteralInteger },
		{ std::wregex{ LR"(\d+\.\d+)" }, TokenType::LiteralDecimal },
		{ std::wregex{ LR"(\w+)" }, TokenType::Identifier },
		{ std::wregex{ LR"(<|>|<>|<=|>=|=)" }, TokenType::OperatorComparison },
		{ std::wregex{ LR"(\+|-|\*|/|%)" }, TokenType::OperatorArithmetic },
		{ std::wregex{ LR"(\*\*|\+\+|--)" }, TokenType::OperatorBitwise },
	};
	
	auto trimmed = Trim(lexeme);

	if (trimmed.empty())
	{
		return;
	}

	if (trimmed.length() == 1)
	{
		if (AddToken(trimmed[0], tokens))
		{
			return;
		}
	}

	for (const auto& pair : tokenRegexes)
	{
		if (std::regex_match(trimmed.begin(), trimmed.end(), pair.first))
		{
			tokens.emplace_back(Token{
				.type = pair.second,
				.value{ trimmed.data(), trimmed.size() }
				});
			return;
		}
	}

	errors.emplace_back(Error{ .code = Error::Code::SyntaxError, .message = L"Syntax error near \"" + std::wstring(trimmed.data(), trimmed.size()) + L"\"." });
}

std::vector<Token> Lexer::Lex(std::wistream& charStream) noexcept
{
	static constexpr std::wstring_view delimiters{ L"()[]{}.,:;!?" };

	std::vector<Token> tokens;
	std::wstring lexeme;

	struct State
	{
		bool skipNext = false;
		bool integerLiteral = false;
		bool stringLiteral = false;
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

				AddToken(lexeme, tokens);
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
		else if (state.stringLiteral)
		{
			if (c == '\'')
			{
				if (c == next)
				{
					// Keep a single escaped quotation mark
					lexeme += c;
					state.skipNext = true;
					goto readNext;
				}
				else
				{
					tokens.emplace_back(Token{
						.type = TokenType::LiteralString,
						.value = lexeme
						});
					lexeme.clear();

					state.stringLiteral = false;
					goto readNext;
				}
			}
		}
		else if (c == '\'')
		{
			AddToken(lexeme, tokens);
			lexeme.clear();
			state = State{};

			state.stringLiteral = true;
			goto readNext;
		}
		else if (state.integerLiteral && c == '.' && std::iswdigit(next))
		{
			// Decimal number literal
			state.integerLiteral = false;
		}
		else if (std::iswdigit(c) && lexeme.empty())
		{
			state.integerLiteral = true;
		}
		else if (std::iswspace(c))
		{
			AddToken(lexeme, tokens);
			lexeme.clear();
			state = State{};
		}
		else if (delimiters.find(c) != delimiters.npos)
		{
			AddToken(lexeme, tokens);
			lexeme.clear();
			state = State{};

			AddToken(c, tokens);
			goto readNext;
		}

		if (!std::iswspace(c))
		{
			lexeme += c;
		}

	readNext:
		prev = c;
		c = next;
	}

	if (!state.commentLevel)
	{
		AddToken(lexeme, tokens);
		lexeme.clear();
	}
	else
	{
		errors.emplace_back(Error{ .code = Error::Code::CommentBracketMismatch, .message = L"Mismatch in the number of opening and closing double angle quotation marks for nested comments." });
	}

	return tokens;
}
