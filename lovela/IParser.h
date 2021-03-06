#pragma once
#include "Node.h"

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

		constexpr bool operator<=>(const Error& rhs) const noexcept = default;
	};

	[[nodiscard]] virtual std::unique_ptr<Node> Parse() noexcept = 0;
	[[nodiscard]] virtual const std::vector<Error>& GetErrors() noexcept = 0;
};
