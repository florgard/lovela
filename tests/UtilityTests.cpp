#include "pch.h"

#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp"
using namespace boost::ut;

suite is_int_tests = [] {
	static_assert(is_int("0"));
	static_assert(is_int("1"));
	static_assert(is_int("-1"));
	static_assert(is_int("+1"));
	static_assert(is_int("+1"));
	static_assert(is_int("001"));
	static_assert(is_int("-001"));
	static_assert(is_int("123"));
	static_assert(is_int("2147483647"));
	static_assert(is_int("-2147483648"));
	static_assert(is_int("4294967295"));
	static_assert(is_int("9223372036854775807"));
	static_assert(is_int("-9223372036854775808"));
	static_assert(is_int("18446744073709551615"));
	static_assert(is_int("20000000000000000000")); // is_int currently doesn't stop at any boundary. May change.
	static_assert(is_int("-20000000000000000000")); // is_int currently doesn't stop at any boundary. May change.

	static_assert(!is_int(""));
	static_assert(!is_int(" "));
	static_assert(!is_int("a"));
	static_assert(!is_int(" 1"));
	static_assert(!is_int("1 "));
	static_assert(!is_int("1a"));
	static_assert(!is_int("a1"));
	static_assert(!is_int("-1a"));
	static_assert(!is_int("+1a"));
};

suite to_int_tests = [] {

};

suite static_map_tests = [] {
	static constexpr std::array<std::pair<int, double>, 2> values{ { {1, 1.1}, {2, 2.2} } };
	static constexpr auto map = static_map<int, double, values.size()>{ {values} };

	static_assert(map.at(1) == 1.1);
	static_assert(map.at(2) == 2.2);
	static_assert(map.at_or(2, 1.1) == 2.2);
	static_assert(map.at_or(3, 1.1) == 1.1);

	"Out of bounds"_test = [] {
		expect(throws([] { auto x = map.at(3); x; }));
	};
};

suite split_tests = [] {
	"split"_test = [] {
		std::wstring s(L"a b c");
		auto v = to_vector(split(s, L' '));
		expect(v.size() == 3);
		expect(v[1] == L"b");
	};

	"co_split string"_test = [] {
		std::string s("a b c");
		auto v = to_vector(co_split(s, ' '));
		expect(v.size() == 3);
		expect(v[1] == "b");
	};

	"co_wsplit wstring"_test = [] {
		std::wstring s(L"a b c");
		auto v = to_vector(co_wsplit(s, L' '));
		expect(v.size() == 3);
		expect(v[1] == L"b");
	};

	"co_split string move"_test = [] {
		std::string s("a b c");
		auto v = to_vector(co_split(std::move(s), ' '));
		expect(v.size() == 3);
		expect(v[1] == "b");
	};

	"co_wsplit wstring move"_test = [] {
		std::wstring s(L"a b c");
		auto v = to_vector(co_wsplit(std::move(s), L' '));
		expect(v.size() == 3);
		expect(v[1] == L"b");
	};

	"co_split istringstream"_test = [] {
		std::string s("a b c");
		std::istringstream ss(std::move(s));
		auto v = to_vector(co_split(std::move(ss), ' '));
		expect(v.size() == 3);
		expect(v[1] == "b");
	};

	"co_wsplit wistringstream"_test = [] {
		std::wstring s(L"a b c");
		std::wistringstream ss(std::move(s));
		auto v = to_vector(co_wsplit(std::move(ss), L' '));
		expect(v.size() == 3);
		expect(v[1] == L"b");
	};
};
