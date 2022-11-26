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
		size_t line = 1;
		size_t column = 1;
		std::string message;

		[[nodiscard]] constexpr bool operator==(const Error& rhs) const noexcept
		{
			// Compare the error line only on error. Don't compare the error column and message.
			return code == rhs.code && (code == LexerError::NoError || line == rhs.line);
		}

		[[nodiscard]] constexpr bool operator!=(const Error& rhs) const noexcept
		{
			return !operator==(rhs);
		}
	} error;

	[[nodiscard]] constexpr auto operator<=>(const Token& rhs) const noexcept = default;

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
		stream << '[' << to_string(type) << ',' << value << ',' << to_string(error.code) << ',' << error.line << ',' << error.column << ',' << error.message << ']';
	}
};
