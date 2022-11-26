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

	std::string value;

	struct Error
	{
		LexerError code{};
		std::string_view message;
		size_t line{};
		size_t column{};
	} error;

	[[nodiscard]] constexpr bool operator==(const Token& rhs) const noexcept
	{
		if (type == Type::Error || rhs.type == Type::Error)
		{
			// Don't compare the value (containing the error message). Don't compare the code location.
			return rhs.type == type && rhs.error.code == error.code;
		}
		else
		{
			return rhs.type == type && rhs.value == value;
		}
	}

	[[nodiscard]] constexpr bool operator!=(const Token& rhs) const noexcept
	{
		return !operator==(rhs);
	}

	[[nodiscard]] constexpr operator bool() const noexcept
	{
		return type != Type::Empty;
	}

	[[nodiscard]] constexpr bool IsError() const noexcept
	{
		return type == Type::Error;
	}

	[[nodiscard]] void Print(std::ostream& stream) const
	{
		stream << '[' << to_string(type) << ',' << to_string(error.code) << ',' << value << ']';
	}
};
