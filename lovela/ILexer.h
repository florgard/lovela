#pragma once
#include <string>
#include <vector>
#include <experimental/generator>
#include "Token.h"

using TokenGenerator = std::experimental::generator<Token>;

class ILexer
{
public:
	struct Error
	{
		enum class Code
		{
			UnknownError,
			SyntaxError,
			CommentOpen,
			StringLiteralOpen,
			StringFieldIllformed,
			StringFieldUnknown,
			StringInterpolationOverflow,
		} code{};

		int line{};
		int column{};
		std::wstring message;

		constexpr bool operator<=>(const Error& rhs) const noexcept = default;
	};

	virtual TokenGenerator Lex() noexcept = 0;
	virtual const std::vector<Error>& GetErrors() noexcept = 0;
};
