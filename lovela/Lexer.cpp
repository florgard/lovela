#include "Lexer.h"
#include <string>
#include <string_view>
#include <map>
#include <regex>
#include <array>
#include <algorithm>
#include <iostream>
#include <cwctype>

using namespace std::literals::string_view_literals;

constexpr std::wstring_view Trim(const std::wstring_view& input) noexcept
{
	constexpr wchar_t whitespace[]{ L" \t\r\n\f\v" };

	const auto start = input.find_first_not_of(whitespace);
	if (start == input.npos)
	{
		return {};
	}

	const auto end = input.find_last_not_of(whitespace);
	return input.substr(start, end - start + 1);
}

static_assert(Trim(L""sv) == L""sv);
static_assert(Trim(L" \t\r\n\f\v"sv) == L""sv);
static_assert(Trim(L"a b c \r\n"sv) == L"a b c"sv);

constexpr TokenType GetTokenType(wchar_t lexeme) noexcept
{
	constexpr std::array<std::pair<wchar_t, TokenType>, 13> characters{ {
		{'(', TokenType::ParenRoundOpen },
		{')', TokenType::ParenRoundClose },
		{'[', TokenType::ParenSquareOpen },
		{']', TokenType::ParenSquareClose },
		{'{', TokenType::ParenCurlyOpen },
		{'}', TokenType::ParenCurlyClose },
		{'.', TokenType::SeparatorDot },
		{',', TokenType::SeparatorComma },
		{'!', TokenType::SeparatorExclamation },
		{'?', TokenType::SeparatorQuestion },
		{':', TokenType::OperatorColon },
	} };

	auto iter = std::find_if(characters.begin(), characters.end(), [&](const auto& pair) { return pair.first == lexeme; });
	return (iter != characters.end()) ? iter->second : TokenType::Unknown;
}

static_assert(GetTokenType('(') == TokenType::ParenRoundOpen);
static_assert(GetTokenType('.') == TokenType::SeparatorDot);
static_assert(GetTokenType(' ') == TokenType::Unknown);

constexpr bool AddToken(wchar_t lexeme, std::vector<Token>& tokens) noexcept
{
	auto type = GetTokenType(lexeme);
	if (type == TokenType::Unknown)
	{
		return false;
	}

	tokens.emplace_back(Token{ .type = type, .value = std::wstring(1, lexeme) });

	return true;
}

void AddToken(const std::wstring_view& lexeme, std::vector<Token>& tokens)
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

	std::wcerr << "Syntax error near \"" << trimmed << "\".\n";
}

std::vector<Token> Lexer2::Lex(std::wistream& charStream) noexcept
{
	static constexpr std::wstring_view delimiters{ L"()[]{}.,:;!?" };

	std::vector<Token> tokens;
	std::wstring lexeme;

	struct State
	{
		bool skipNext = false;
		bool inNumberLiteral = false;
		bool inStringLiteral = false;
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
		else if (state.inStringLiteral)
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

					state.inStringLiteral = false;
					goto readNext;
				}
			}
		}
		else if (c == '\'')
		{
			AddToken(lexeme, tokens);
			lexeme.clear();
			state = State{};

			state.inStringLiteral = true;
			goto readNext;
		}
		else if (state.inNumberLiteral && c == '.')
		{
			// Decimal literal, don't break the lexeme.
		}
		else if (std::iswdigit(c) && lexeme.empty())
		{
			state.inNumberLiteral = true;
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
		std::cerr << "Mismatch in the number of opening and closing double angle quotation marks for nested comments.\n";
	}

	return tokens;
}
