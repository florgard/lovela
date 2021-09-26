export module IParser;

import Token;
import Node;
import <string>;
import <vector>;
import <memory>;

export
{
	class IParser
	{
	public:
		struct Error
		{
			enum class Code
			{
				NoError,
				ParseError,
			} code{};

			std::wstring message;
			Token token;

			[[nodiscard]] constexpr auto operator<=>(const Error& rhs) const noexcept = default;
		};

		[[nodiscard]] virtual std::unique_ptr<Node> Parse() noexcept = 0;
		[[nodiscard]] virtual const std::vector<Error>& GetErrors() noexcept = 0;
	};
}