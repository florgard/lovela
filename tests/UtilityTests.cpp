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
