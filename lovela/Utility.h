#pragma once

[[nodiscard]] inline std::wstring to_wstring(std::string_view value)
{
	return std::wstring(value.begin(), value.end());
}

template <typename Enum, typename std::enable_if<std::is_enum_v<Enum>>::type* = nullptr>
[[nodiscard]] inline std::wstring to_wstring(Enum value)
{
	return to_wstring(magic_enum::enum_name(value));
}

// https://www.reddit.com/r/cpp/comments/g05m1r/stdunique_ptr_and_braced_initialization/
template <class T>
std::unique_ptr<T> clone_unique(T t)
{
	return std::make_unique<T>(std::move(t));
}
