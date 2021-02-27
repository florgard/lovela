#pragma once
#include <string>

enum class TokenType
{
	Empty,
	Error,
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
	constexpr operator bool() const noexcept { return type != TokenType::Empty; };
};
