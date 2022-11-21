#pragma once

struct Token
{
	using StringT = std::string;

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
		SeparatorSlash,
		SeparatorColon,
		SeparatorHash,
		OperatorComparison,
		OperatorArithmetic,
		OperatorBitwise,
		OperatorArrow,
	} type{};

	StringT value;
	StringT outType;

	int line{};
	int column{};
	StringT code;

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
