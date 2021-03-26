#pragma once

struct Token
{
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

	[[nodiscard]] bool operator==(const Token& rhs) const noexcept;
	[[nodiscard]] bool operator!=(const Token& rhs) const noexcept
	{
		return !operator==(rhs);
	}

	[[nodiscard]] operator bool() const noexcept { return type != Type::Empty; }
};
