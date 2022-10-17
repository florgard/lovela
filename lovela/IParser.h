#pragma once
#include "INodeGenerator.h"

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

	virtual ~IParser() = default;

	[[nodiscard]] virtual INodeGenerator Parse() noexcept = 0;
	[[nodiscard]] virtual const std::vector<Error>& GetErrors() noexcept = 0;
};
