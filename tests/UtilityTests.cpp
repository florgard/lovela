#include "pch.h"

suite static_map_tests = [] {
	static constexpr std::array<std::pair<int, double>, 2> values{ { {1, 1.1}, {2, 2.2} } };
	static constexpr auto map = static_map<int, double, values.size()>{ {values} };

	static_assert(map.at(1) == 1.1);
	static_assert(map.at(2) == 2.2);
	static_assert(map.at_or(2, 1.1) == 2.2);
	static_assert(map.at_or(3, 1.1) == 1.1);

	"Out of bounds"_test = [] { expect(throws([] { auto x = map.at(3); x; })); };
};