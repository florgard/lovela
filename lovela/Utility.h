#pragma once

[[nodiscard]] inline std::wstring to_wstring(std::string_view value)
{
	return std::wstring(value.begin(), value.end());
}

[[nodiscard]] inline std::wstring to_wstring(std::wstring_view value)
{
	return std::wstring(value.begin(), value.end());
}

template <typename Enum, typename std::enable_if<std::is_enum_v<Enum>>::type* = nullptr>
[[nodiscard]] std::wstring to_wstring(Enum value)
{
	return to_wstring(magic_enum::enum_name(value));
}

// https://www.reddit.com/r/cpp/comments/g05m1r/stdunique_ptr_and_braced_initialization/
// Moves r-value to unique_ptr, copies l-value into unique_ptr.
template <class T>
[[nodiscard]] std::unique_ptr<T> clone_unique(T t)
{
	return std::make_unique<T>(std::move(t));
}
// Moves r-value into unique_ptr.
template <class T>
[[nodiscard]] std::unique_ptr<T> move_to_unique(T&& t) {
	return std::make_unique<T>(std::move(t));
}
// Moves l-value into unique_ptr.
template <class T>
[[nodiscard]] std::unique_ptr<T> move_to_unique(T& t) {
	return std::make_unique<T>(std::move(t));
}

// Moves r-value into shared_ptr.
template <class T>
[[nodiscard]] std::shared_ptr<T> move_to_shared(T&& t) {
	return std::make_shared<T>(std::move(t));
}
// Moves l-value into shared_ptr.
template <class T>
[[nodiscard]] std::shared_ptr<T> move_to_shared(T& t) {
	return std::make_shared<T>(std::move(t));
}

template <typename T>
struct make
{
	[[nodiscard]] static std::unique_ptr<T> unique() noexcept { return std::make_unique<T>(); }
	[[nodiscard]] static std::unique_ptr<T> unique(T& src) noexcept { return move_to_unique<T>(src); }
	[[nodiscard]] static std::unique_ptr<T> unique(T&& src) noexcept { return move_to_unique<T>(src); }
	[[nodiscard]] static std::shared_ptr<T> shared() noexcept { return std::make_shared<T>(); }
	[[nodiscard]] static std::shared_ptr<T> shared(T& src) noexcept { return move_to_shared<T>(src); }
	[[nodiscard]] static std::shared_ptr<T> shared(T&& src) noexcept { return move_to_shared<T>(src); }
};
