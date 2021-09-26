export module Parser.DataType;

import <string>;

export struct DataType
{
	std::wstring name;

	[[nodiscard]] auto operator<=>(const DataType& rhs) const noexcept = default;
};
