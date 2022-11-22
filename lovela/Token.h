#pragma once

struct Token
{
	struct Constant
	{
		static constexpr std::string_view TypeNameSpace = "type";
		static constexpr std::string_view NameSpaceSeparator = "/";
	};

	enum class Type
	{
		Empty,
		End,
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
		SeparatorVerticalLine,
		SeparatorSlash,
		SeparatorColon,
		SeparatorHash,
		OperatorComparison,
		OperatorArithmetic,
		OperatorBitwise,
		OperatorArrow,
	} type{};

	std::string value;
	std::string outType;

	int line{};
	int column{};
	std::string code;

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
