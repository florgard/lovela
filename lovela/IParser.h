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

		int line{};
		int column{};
		std::wstring message;

		constexpr bool operator<=>(const Error& rhs) const noexcept = default;
	};

	[[nodiscard]] virtual Node Parse() noexcept = 0;
	[[nodiscard]] virtual const std::vector<Error>& GetErrors() noexcept = 0;
};
