#pragma once

struct DataType
{
	std::wstring name;

	[[nodiscard]] auto operator<=>(const DataType& rhs) const noexcept = default;
};
