#pragma once

struct Token
{
	enum class Type
	{
		Empty,
		End,
		Identifier,
		PrimitiveType,
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
		SeparatorVerticalLine,
		SeparatorColon,
		SeparatorHash,
		OperatorComparison,
		OperatorArithmetic,
		OperatorBitwise,
		OperatorArrow,
	} type{};

	std::wstring value;
	std::wstring outType;

	int line{};
	int column{};
	std::wstring code;

	[[nodiscard]] constexpr bool operator==(const Token& rhs) const noexcept
	{
		// Compare everything but the code location
		return rhs.type == type && rhs.value == value;
	}

	[[nodiscard]] constexpr bool operator!=(const Token& rhs) const noexcept
	{
		return !operator==(rhs);
	}

	[[nodiscard]] constexpr operator bool() const noexcept
	{
		return type != Type::Empty;
	}
};
