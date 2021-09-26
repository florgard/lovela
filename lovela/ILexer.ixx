export module ILexer;

import Lexer.Token;
import <string>;
import <vector>;
import <memory>;
import <experimental\generator>;

export using TokenGenerator = std::experimental::generator<Token>;
//export using TokenIterator = std::experimental::generator<Token>::iterator;

export class ILexer
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
