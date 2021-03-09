#pragma once

[[nodiscard]] inline std::wstring ToWString(std::string_view value)
{
	return std::wstring(value.begin(), value.end());
}

template <typename Enum, typename std::enable_if<std::is_enum_v<Enum>>::type* = nullptr>
[[nodiscard]] inline std::wstring ToWString(Enum value)
{
	return ToWString(magic_enum::enum_name(value));
}
