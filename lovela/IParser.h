#pragma once
#include "Node.h"

using NodeGenerator = tl::generator<Node>;

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

		std::string message;
		Token token;

		[[nodiscard]] constexpr auto operator<=>(const Error& rhs) const noexcept = default;
	};

	virtual ~IParser() = default;

	[[nodiscard]] virtual NodeGenerator Parse() noexcept = 0;
	[[nodiscard]] virtual const std::vector<Error>& GetErrors() const noexcept = 0;
};
