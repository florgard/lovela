#include "Lexer.h"
#include <string>
#include <string_view>
#include <map>
#include <regex>
#include <array>
#include <algorithm>
#include <iostream>

using namespace std::literals::string_view_literals;

constexpr std::string_view Trim(const std::string_view& input) noexcept
{
	constexpr char whitespace[]{ " \t\r\n\f\v" };

	const auto start = input.find_first_not_of(whitespace);
	if (start == input.npos)
	{
		return "";
	}

	const auto end = input.find_last_not_of(whitespace);
	return input.substr(start, end - start + 1);
}

static_assert(Trim(""sv) == ""sv);
static_assert(Trim(" \t\r\n\f\v"sv) == ""sv);
static_assert(Trim("a b c \r\n"sv) == "a b c"sv);

constexpr TokenType GetTokenType(char lexeme) noexcept
{
	constexpr std::array<std::pair<char, TokenType>, 13> characters{ {
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

constexpr bool AddToken(char lexeme, std::vector<Token>& tokens) noexcept
{
	auto type = GetTokenType(lexeme);
	if (type == TokenType::Unknown)
	{
		return false;
	}

	tokens.emplace_back(Token{ .type = type, .value = std::string(1, lexeme) });

	return true;
}

void AddToken(const std::string_view& lexeme, std::vector<Token>& tokens)
{
	static const std::vector<std::pair<std::regex, TokenType>> tokenRegexes{
		{ std::regex{ R"(\d+)" }, TokenType::LiteralInteger },
		{ std::regex{ R"(\d+\.\d+)" }, TokenType::LiteralDecimal },
		{ std::regex{ R"(\w+)" }, TokenType::Identifier },
		{ std::regex{ R"(<|>|<>|<=|>=|=)" }, TokenType::OperatorComparison },
		{ std::regex{ R"(\+|-|\*|/|%)" }, TokenType::OperatorArithmetic },
		{ std::regex{ R"(\*\*|\+\+|--)" }, TokenType::OperatorBitwise },
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

	std::cerr << "Syntax error near \"" << trimmed << "\".\n";
}

std::vector<Token> Lex(std::istream& charStream) noexcept
{
	static constexpr std::string_view delimiters{ "()[]{}.,:;!?" };

	std::vector<Token> tokens;
	std::string lexeme;

	struct State
	{
		bool inNumberLiteral = false;
		bool inStringLiteral = false;
		char quotationMark = 0;
		bool inOpenComment = false;
		bool inCloseComment = false;
		int commentLevel = 0;
	} state;

	char c, prev = 0;

	auto& stream = charStream >> std::noskipws;

	while (stream >> c)
	{
		if (c == '<')
		{
			if (state.inOpenComment)
			{
				// Still opening comment
				prev = c;
				continue;
			}
			else if (c == prev)
			{
				// Begin opening comment
				state.inOpenComment = true;
				state.inCloseComment = false;
				state.commentLevel++;

				lexeme.clear();
				prev = c;
				continue;
			}
			else if (!state.commentLevel)
			{
				AddToken(lexeme, tokens);
				lexeme.clear();

				lexeme += c;
				prev = c;
				continue;
			}
		}
		else
		{
			state.inOpenComment = false;
		}

		if (c == '>')
		{
			if (state.inCloseComment)
			{
				// Still closing comment
				prev = c;
				continue;
			}
			else if (c == prev)
			{
				// Begin closing comment
				state.inOpenComment = false;
				state.inCloseComment = true;
				state.commentLevel--;

				lexeme.clear();
				prev = c;
				continue;
			}
			else if (!state.commentLevel)
			{
				AddToken(lexeme, tokens);
				lexeme.clear();

				lexeme += c;
				prev = c;
				continue;
			}
		}
		else
		{
			state.inCloseComment = false;
		}

		if (state.commentLevel)
		{
			// Consume the comment
			prev = c;
			continue;
		}
		else if (state.inStringLiteral)
		{
			if (c == state.quotationMark)
			{
				tokens.emplace_back(Token{
					.type = TokenType::LiteralString,
					.value = lexeme
					});

				state.inStringLiteral = false;
				state.quotationMark = 0;
				prev = c;
				continue;
			}
		}
		else if (c == '\'' || c == '"')
		{
			AddToken(lexeme, tokens);
			lexeme.clear();
			state = State{};

			state.inStringLiteral = true;
			state.quotationMark = c;
			prev = c;
			continue;
		}
		else if (state.inNumberLiteral && c == '.')
		{
			// Decimal literal, don't break the lexeme.
		}
		else if (std::isdigit(c) && lexeme.empty())
		{
			state.inNumberLiteral = true;
		}
		else if (std::isspace(c))
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
			prev = c;
			continue;
		}

		if (!std::isspace(c))
		{
			lexeme += c;
		}

		prev = c;
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
