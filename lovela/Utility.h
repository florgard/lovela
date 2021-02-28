#pragma once
#include <string>

inline std::wstring ToWString(std::string_view value)
{
	return std::wstring(value.begin(), value.end());
}
