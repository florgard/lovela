#pragma once

[[nodiscard]] inline std::wstring ToWString(std::string_view value)
{
	return std::wstring(value.begin(), value.end());
}

template <typename T, typename U>
[[nodiscard]] constexpr bool contains(const T& container, const U& value)
{
	return std::find(std::begin(container), std::end(container), value) != std::end(container);
}
