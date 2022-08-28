import Utility.StaticMap;
import <string>;
import <array>;
import <vector>;

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_USE_STD_HEADERS
#include "doctest.h"

TEST_CASE("test StaticMap") {
	static constexpr std::array<std::pair<int, double>, 2> values{ { {1, 1.1}, {2, 2.2} } };
	static constexpr auto map = StaticMap<int, double, values.size()>{ {values} };

	static_assert(map.at(1) == 1.1);
	static_assert(map.at(2) == 2.2);
	static_assert(map.at_or(2, 1.1) == 2.2);
	static_assert(map.at_or(3, 1.1) == 1.1);

	SUBCASE("Out of bounds") {
		CHECK_THROWS(map.at(3) == 3.3);
	};
}
