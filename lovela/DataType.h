#pragma once

struct DataType
{
	std::string name;

	[[nodiscard]] auto operator<=>(const DataType& rhs) const noexcept = default;
};
