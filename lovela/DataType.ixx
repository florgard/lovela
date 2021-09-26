export module Parser.DataType;

import <string>;
import <string_view>;
import <iostream>;
import <sstream>;
import <vector>;
import <array>;
import <set>;
import <map>;
import <functional>;
import <algorithm>;

export struct DataType
{
	std::wstring name;

	[[nodiscard]] auto operator<=>(const DataType& rhs) const noexcept = default;
};
