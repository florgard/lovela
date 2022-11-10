#pragma once

template <typename CharT>
struct basic_token
{
	using TokenT = basic_token<CharT>;
	using StringT = std::basic_string<CharT>;

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

	StringT value;
	StringT outType;

	int line{};
	int column{};
	StringT code;

	[[nodiscard]] constexpr bool operator==(const TokenT& rhs) const noexcept
	{
		// Compare everything but the code location
		return rhs.type == type && rhs.value == value;
	}

	[[nodiscard]] constexpr bool operator!=(const TokenT& rhs) const noexcept
	{
		return !operator==(rhs);
	}

	[[nodiscard]] constexpr operator bool() const noexcept
	{
		return type != Type::Empty;
	}
};

// FIXME: Use char
using Token = basic_token<wchar_t>;
using TokenW = basic_token<wchar_t>;
