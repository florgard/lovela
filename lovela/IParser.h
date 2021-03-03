#pragma once
#include "Node.h"

class IParser
{
public:
	struct Error
	{
		enum class Code
		{
			ParseError,
		} code{};

		std::wstring message;

		constexpr bool operator<=>(const Error& rhs) const noexcept = default;
	};

	[[nodiscard]] virtual Node Parse() noexcept = 0;
};
