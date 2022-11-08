#pragma once
#include "magic_enum.hpp"

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

[[nodiscard]] inline std::wstring single_quote(const std::wstring& text)
{
	return L'\'' + text + L'\'';
}

[[nodiscard]] inline std::wstring double_quote(const std::wstring& text)
{
	return L'"' + text + L'"';
}

template <typename Container>
[[nodiscard]] auto join(const Container& input, std::string_view delimiter)
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
	std::vector<std::wstring> tokens;
	std::wistringstream ss(input);
	std::wstring token;

	while (std::getline(ss, token, delimiter))
	{
		tokens.emplace_back(token);
	}

	return tokens;

	// error LNK2019: unresolved external symbol "public: struct std::experimental::generator<...
	//return [=]() -> std::experimental::generator<std::wstring>
	//{
	//	std::wistringstream ss(input);
	//	std::wstring token;

	//	while (std::getline(ss, token, delimiter))
	//	{
	//		co_yield token;
	//	}
	//};
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

#pragma once

// Clone of Jason Turner's constexpr map: https://www.youtube.com/watch?v=INn3xa4pMfg
// Usage:
// static constexpr std::array<std::pair<int, double>, 2> values{ { {1, 1.1}, {2, 2.2} } };
// static constexpr auto map = static_map<int, double, values.size()>{ {values} };
// static_assert(map.at(2) == 2.2);
template <typename Key, typename Value, std::size_t Size>
struct static_map
{
	std::array<std::pair<Key, Value>, Size> data;

	[[nodiscard]] constexpr Value at(const Key& key) const
	{
		const auto itr = std::find_if(data.begin(), data.end(), [&key](const auto& v) { return v.first == key; });
		if (itr != data.end())
		{
			return itr->second;
		}
		else
		{
			throw std::range_error("Not Found");
		}
	}

	[[nodiscard]] constexpr Value at_or(const Key& key, Value defaultValue) const
	{
		const auto itr = std::find_if(data.begin(), data.end(), [&key](const auto& v) { return v.first == key; });
		return (itr != data.end()) ? itr->second : defaultValue;
	}
};

// Usage:
// static constexpr std::array<int>, 2> values{ 1, 3 };
// static constexpr auto set = static_set<int, values.size()>{ {values} };
// static_assert(map.contains(3));
template <typename Key, std::size_t Size>
struct static_set
{
	std::array<Key, Size> data;

	[[nodiscard]] constexpr bool contains(const Key& key) const
	{
		const auto itr = std::find(data.begin(), data.end(), key);
		return itr != data.end();
	}

	[[nodiscard]] constexpr bool contains_if(auto predicate) const
	{
		const auto itr = std::find_if(data.begin(), data.end(), [&](Key key) { return predicate(key); });
		return itr != data.end();
	}
};
