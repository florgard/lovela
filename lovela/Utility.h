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

template <typename T, typename std::enable_if<!std::is_enum_v<T>>::type* = nullptr>
[[nodiscard]] std::wstring to_wstring(T value)
{
	std::wostringstream s;
	s << value;
	return s.str();
}

inline std::wstring single_quote(const std::wstring& text)
{
	return L'\'' + text + L'\'';
}

inline std::wstring double_quote(const std::wstring& text)
{
	return L'"' + text + L'"';
}

inline auto split(const std::wstring& input, wchar_t delimiter)
{
	return [=]() -> std::experimental::generator<std::wstring>
	{
		std::wistringstream ss(input);
		std::wstring token;

		while (std::getline(ss, token, delimiter))
		{
			co_yield token;
		}
	};
}

// https://www.reddit.com/r/cpp/comments/g05m1r/stdunique_ptr_and_braced_initialization/
template <typename T>
struct make
{
	// Makes a unique_ptr with a default constructed object.
	[[nodiscard]] static std::unique_ptr<T> unique() noexcept { return std::make_unique<T>(); }
	// Moves an l-value into unique_ptr.
	[[nodiscard]] static std::unique_ptr<T> unique(T& src) noexcept { return std::make_unique<T>(std::move(src)); }
	// Moves an r-value into unique_ptr.
	[[nodiscard]] static std::unique_ptr<T> unique(T&& src) noexcept { return std::make_unique<T>(std::move(src)); }
	// Makes a shared_ptr with a default constructed object.
	[[nodiscard]] static std::shared_ptr<T> shared() noexcept { return std::make_shared<T>(); }
	// Moves an l-value into shared_ptr.
	[[nodiscard]] static std::shared_ptr<T> shared(T& src) noexcept { return std::make_shared<T>(std::move(src)); }
	// Moves an r-value into shared_ptr.
	[[nodiscard]] static std::shared_ptr<T> shared(T&& src) noexcept { return std::make_shared<T>(std::move(src)); }
};
