#pragma once
#include "Token.h"

using TokenGenerator = std::experimental::generator<Token>;

class ILexer
{
public:
	struct Error
	{
		enum class Code
		{
			NoError,
			SyntaxError,
			CommentOpen,
			StringLiteralOpen,
			StringFieldIllformed,
			StringFieldUnknown,
			StringInterpolationOverflow,
		} code{};

		std::wstring message;
		Token token;

		[[nodiscard]] auto operator<=>(const Error& rhs) const noexcept = default;
	};

	[[nodiscard]] virtual TokenGenerator Lex() noexcept = 0;
	[[nodiscard]] virtual const std::vector<Error>& GetErrors() noexcept = 0;
};
