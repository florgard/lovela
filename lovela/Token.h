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

	bool operator<=>(const Token& rhs) const noexcept = default;
	bool operator!=(const Token& rhs) const noexcept = default;
	operator bool() const noexcept { return type != Type::Empty; };
};
