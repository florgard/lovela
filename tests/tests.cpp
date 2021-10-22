import LexerBase;
//import LexerFactory;
//import Parser.Node;
//import IParser;
//import ParserFactory;
//import CodeGeneratorFactory;
//import Utility;
//import Utility.StaticMap;
import <utility>;
import <string>;
import <string_view>;
import <array>;
import <vector>;
import <regex>;

#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp"

using namespace boost::ut;

suite static_map = [] {
	static constexpr std::array<std::pair<int, double>, 2> values{ { {1, 1.1}, {2, 2.2} } };
	static constexpr auto map = StaticMap<int, double, values.size()>{ {values} };

	static_assert(map.at(1) == 1.1);
	static_assert(map.at(2) == 2.2);
	static_assert(map.at_or(2, 1.1) == 2.2);
	static_assert(map.at_or(3, 1.1) == 1.1);

	"Out of bounds"_test = [] {
		expect(throws([] { if (map.at(3) == 3.3) {} }));
	};
};

int main()
{
}
