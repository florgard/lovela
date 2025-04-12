#pragma once

struct Defer final
{
	Defer(std::function<void()> deferred) : m_Deferred(deferred) { }
	~Defer() { m_Deferred(); }

private:
	std::function<void()> m_Deferred;
};

template<typename T>
concept Clearable = requires(T t)
{
	{ t.clear() };
};

template<Clearable T>
[[nodiscard]] static constexpr T MoveClear(T& value)
{
	T moved = std::move(value);
	value.clear();
	return moved;
}

[[nodiscard]] static constexpr std::string_view Trim(const std::string_view& input) noexcept
{
	constexpr std::array<char, 7> whitespace{ ' ', '\t', '\r', '\n', '\f', '\v', '\0' };

	const auto start = input.find_first_not_of(whitespace.data());
	if (start == input.npos)
		return {};

	const auto end = input.find_last_not_of(whitespace.data(), input.npos, whitespace.size());
	return input.substr(start, end - start + 1);
}

[[nodiscard]] static constexpr std::wstring_view Trim(const std::wstring_view& input) noexcept
{
	constexpr std::array<wchar_t, 7> whitespace{ L' ', L'\t', L'\r', L'\n', L'\f', L'\v', L'\0' };

	const auto start = input.find_first_not_of(whitespace.data());
	if (start == input.npos)
		return {};

	const auto end = input.find_last_not_of(whitespace.data(), input.npos, whitespace.size());
	return input.substr(start, end - start + 1);
}

[[nodiscard]] constexpr bool is_int(std::string_view value) noexcept
{
	if (value.empty())
		return false;

	size_t i = 0;
	switch (value[0])
	{
	case '-':
		[[fallthrough]];

	case '+':
		++i;
		break;
	}

	size_t digits = 0;
	for (; i < value.length(); ++i)
	{
		const unsigned char digit = value[i] - '0';
		if (digit < 10)
			++digits;
		else
			return false;
	}

	return digits > 0;
}

namespace detail
{
	template <typename T>
	constexpr std::optional<T> to_int_read_value(std::string_view value) noexcept
	{
		// Signed types are expected to represent negative values.
		constexpr bool negative = std::is_signed_v<T>;

		T result = 0;

		for (size_t i = 0; i < value.length(); ++i)
		{
			const unsigned char digit = value[i] - '0';
			if (digit < 10)
			{
				const auto prev = result;

				result *= 10;

				if (result / 10 != prev)
				{
					// Overflow
					return {};
				}

				if (result >= 0)
				{
					if (result > std::numeric_limits<T>::max() - digit)
					{
						// Overflow
						return {};
					}

					result += digit;
				}
				else
				{
					if (result < std::numeric_limits<T>::min() + digit)
					{
						// Overflow
						return {};
					}

					result -= digit;
				}

				if constexpr (negative)
				{
					// Make the result negative at the first non-zero digit to be able to reach max negative integers.
					if (result > 0)
					{
						result = -result;
					}
				}
			}
			else
			{
				return {};
			}
		}

		return result;
	}
}

template <typename S, typename U = std::make_unsigned_t<S>>
[[nodiscard]] constexpr auto to_int(std::string_view value) noexcept
{
	struct return_type
	{
		std::optional<S> signedValue;
		std::optional<U> unsignedValue;

		constexpr bool has_value() const noexcept
		{
			return signedValue.has_value() || unsignedValue.has_value();
		}
	};

	if (value.empty())
		return return_type{};

	bool negative = false;
	size_t i = 0;

	switch (value[0])
	{
	case '-':
		negative = true;
		[[fallthrough]];

	case '+':
		++i;
		break;
	}

	if (negative)
	{
		const auto result = detail::to_int_read_value<S>(value.substr(i));
		return return_type{ result, {} };
	}
	else
	{
		const auto result = detail::to_int_read_value<U>(value.substr(i));
		return return_type{ {}, result };
	}
}

[[nodiscard]] inline std::string to_string(std::string_view value)
{
	return std::string(value.begin(), value.end());
}

[[nodiscard]] inline std::string to_string(std::wstring_view value)
{
	std::string str;
	str.resize(value.length());

	if constexpr (sizeof(wchar_t) == 2)
		utf8::utf16to8(value.begin(), value.end(), str.begin());
	else
		utf8::utf32to8(value.begin(), value.end(), str.begin());

	return str;
}

[[nodiscard]] inline std::wstring to_wstring(std::string_view value)
{
	std::wstring str;
	str.resize(value.length());

	if constexpr (sizeof(wchar_t) == 2)
		utf8::utf8to16(value.begin(), value.end(), str.begin());
	else
		utf8::utf8to32(value.begin(), value.end(), str.begin());

	return str;
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
[[nodiscard]] std::string to_string(Enum value)
{
	return to_string(magic_enum::enum_name(value));
}

template <typename Enum, typename std::enable_if<std::is_enum_v<Enum>>::type* = nullptr>
[[nodiscard]] std::wstring to_wstring(Enum value)
{
	return to_wstring(magic_enum::enum_name(value));
}

template <typename T, typename std::enable_if<!std::is_enum_v<T>>::type* = nullptr>
[[nodiscard]] std::string to_string(const T& value)
{
	std::ostringstream s;
	s << value;
	return s.str();
}

template <typename T, typename std::enable_if<!std::is_enum_v<T>>::type* = nullptr>
[[nodiscard]] std::wstring to_wstring(const T& value)
{
	std::wostringstream s;
	s << value;
	return s.str();
}

[[nodiscard]] inline std::string single_quote(const std::string& text)
{
	return '\'' + text + '\'';
}

[[nodiscard]] inline std::wstring single_quote(const std::wstring& text)
{
	return L'\'' + text + L'\'';
}

[[nodiscard]] inline std::string double_quote(const std::string& text)
{
	return '"' + text + '"';
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

[[nodiscard]] inline auto split(const std::string& input, char delimiter)
{
	std::vector<std::string> tokens;
	std::istringstream ss(input);
	std::string token;

	while (std::getline(ss, token, delimiter))
		tokens.emplace_back(token);

	return tokens;
}

[[nodiscard]] inline auto split(const std::wstring& input, wchar_t delimiter)
{
	std::vector<std::wstring> tokens;
	std::wistringstream ss(input);
	std::wstring token;

	while (std::getline(ss, token, delimiter))
		tokens.emplace_back(token);

	return tokens;
}

[[nodiscard]] inline auto split(std::string&& input, char delimiter)
{
	std::vector<std::string> tokens;
	std::istringstream ss(std::move(input));
	std::string token;

	while (std::getline(ss, token, delimiter))
		tokens.emplace_back(token);

	return tokens;
}

[[nodiscard]] inline auto split(std::wstring&& input, wchar_t delimiter)
{
	std::vector<std::wstring> tokens;
	std::wistringstream ss(std::move(input));
	std::wstring token;

	while (std::getline(ss, token, delimiter))
		tokens.emplace_back(token);

	return tokens;
}

template <typename CharT>
struct basic_co_split
{
	using StringT = std::basic_string<CharT>;
	using StreamT = std::basic_istringstream<CharT>;
	using GeneratorT = tl::generator<StringT>;

	basic_co_split(StreamT&& input, CharT delimiter) noexcept
		: stream(std::move(input))
		, delim(delimiter)
		, gen(get())
	{
	}

	basic_co_split(const StringT& input, CharT delimiter) noexcept
		: stream(input)
		, delim(delimiter)
		, gen(get())
	{
	}

	basic_co_split(StringT&& input, CharT delimiter) noexcept
		: stream(std::move(input))
		, delim(delimiter)
		, gen(get())
	{
	}

	auto begin()
	{
		return gen.begin();
	}

	auto end() const noexcept
	{
		return gen.end();
	}

private:
	GeneratorT get()
	{
		StringT token;

		while (std::getline(stream, token, delim))
		{
			co_yield token;
		}
	}

	StreamT stream;
	GeneratorT gen;
	CharT delim;
};

using co_split = basic_co_split<char>;
using co_wsplit = basic_co_split<wchar_t>;

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
		return (itr != data.end()) ? itr->second : throw std::range_error("Not Found");
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
// static_assert(set.contains(3));
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
