#pragma once
#include "Token.h"

using TokenGenerator = tl::generator<Token>;

class ILexer
{
public:
	struct Error
	{
		enum class Code
		{
			NoError,
			InternalError,
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

	virtual ~ILexer() = default;

	[[nodiscard]] virtual TokenGenerator Lex() noexcept = 0;
	[[nodiscard]] virtual const std::vector<Error>& GetErrors() noexcept = 0;
};
