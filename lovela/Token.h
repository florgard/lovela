#pragma once

#include "LexerError.h"

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
		Error,
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

	LexerError error{};
	std::string value;

	int line{};
	int column{};
	std::string code;

	[[nodiscard]] constexpr bool operator==(const Token& rhs) const noexcept
	{
		// Don't compare the code location. If this is an error token, then also don't compare the value (containing the error message).
		return rhs.type == type && rhs.error == error && (type == Type::Error || rhs.value == value);
	}

	[[nodiscard]] constexpr bool operator!=(const Token& rhs) const noexcept
	{
		return !operator==(rhs);
	}

	[[nodiscard]] constexpr operator bool() const noexcept
	{
		return type != Type::Empty;
	}

	[[nodiscard]] void Print(std::ostream& stream) const
	{
		stream << '[' << to_string(type) << ',' << to_string(error) << ',' << value << ']';
	}
};
