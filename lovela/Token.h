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

	auto operator<=>(const Token& rhs) const noexcept = default;

	// TODO: Replace empty() with operator bool() when the bug in MSVC is fixed (the token tests must pass).
	// The bug is the same or similar as the following, but is present in VS 16.8.6.
	// https://developercommunity.visualstudio.com/t/explicit-operator-bool-can-be-invoked-implicitly-b/635209
	// operator bool() const noexcept { return type != Type::Empty; }
	bool empty() const noexcept { return type == Type::Empty; }
};
