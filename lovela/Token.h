#pragma once
#include <compare>
#include <string>

enum class TokenType
{
	Empty,
	Identifier,
	LiteralString,
	LiteralStringInterpolation,
	LiteralInteger,
	LiteralDecimal,
	ParenRoundOpen,
	ParenRoundClose,
	ParenSquareOpen,
	ParenSquareClose,
	ParenCurlyOpen,
	ParenCurlyClose,
	SeparatorDot,
	SeparatorComma,
	SeparatorExclamation,
	SeparatorQuestion,
	OperatorColon,
	OperatorComparison,
	OperatorArithmetic,
	OperatorBitwise,
};

struct Token
{
	TokenType type;
	std::wstring value;

	constexpr bool operator<=>(const Token& rhs) const noexcept = default;
};
