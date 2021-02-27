#pragma once
#include <string>

struct Token
{
	enum class Type
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
	} type{};

	std::wstring value;

	constexpr bool operator<=>(const Token& rhs) const noexcept = default;
	constexpr operator bool() const noexcept { return type != Type::Empty; };
};
