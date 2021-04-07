#pragma once

[[nodiscard]] inline std::string to_string(std::wstring_view value)
{
	std::string str;
	str.resize(value.length());
	std::transform(value.begin(), value.end(), str.begin(), [](auto elem) { return static_cast<char>(elem); });
	return str;
}

[[nodiscard]] inline std::wstring to_wstring(std::string_view value)
{
	return std::wstring(value.begin(), value.end());
}

[[nodiscard]] inline std::wstring to_wstring(const std::string& value)
{
	return to_wstring(static_cast<std::string_view>(value));
}

[[nodiscard]] inline std::wstring to_wstring(std::wstring_view value)
{
	return std::wstring(value.begin(), value.end());
}

[[nodiscard]] inline const std::wstring& to_wstring(const std::wstring& value)
{
	return value;
}

template <typename Enum, typename std::enable_if<std::is_enum_v<Enum>>::type* = nullptr>
[[nodiscard]] std::wstring to_wstring(Enum value)
{
	return to_wstring(magic_enum::enum_name(value));
}

template <typename T, typename std::enable_if<!std::is_enum_v<T>>::type* = nullptr>
[[nodiscard]] std::wstring to_wstring(const T& value)
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

template <typename Container>
[[nodiscard]] inline auto join(const Container& input, std::string_view delimiter)
{
	std::wostringstream ss;
	int index = 0;
	for (auto& token : input)
	{
		ss << index++ ? delimiter : "";
		ss << to_wstring(token);
	}
	return ss.str();
}

[[nodiscard]] inline auto split(const std::wstring& input, wchar_t delimiter)
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
