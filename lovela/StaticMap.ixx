export module Utility.StaticMap;

import <algorithm>;
import <stdexcept>;

// Clone of Jason Turner's constexpr map: https://www.youtube.com/watch?v=INn3xa4pMfg
// Usage:
// static constexpr std::array<std::pair<int, double>, 2> values{ { {1, 1.1}, {2, 2.2} } };
// static constexpr auto map = StaticMap<int, double, values.size()>{ {values} };
// static_assert(map.at(2) == 2.2);
export template <typename Key, typename Value, std::size_t Size>
struct StaticMap
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
