#include "pch.h"

#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp"
using namespace boost::ut;

suite is_int_tests = [] {
	static_assert(is_int("0"));
	static_assert(is_int("1"));
	static_assert(is_int("+1"));
	static_assert(is_int("-1"));
	static_assert(is_int("001"));
	static_assert(is_int("-001"));
	static_assert(is_int("123"));
	static_assert(is_int("2147483647"), "Max int32 is int");
	static_assert(is_int("-2147483648"), "Min int32 is int");
	static_assert(is_int("4294967295"), "Max uint32 is int");
	static_assert(is_int("9223372036854775807"), "Max int64 is int");
	static_assert(is_int("-9223372036854775808"), "Min int64 is int");
	static_assert(is_int("18446744073709551615"), "Max uint64 is int");
	static_assert(is_int("20000000000000000000"), "Arbitrary big int is int (may change)");
	static_assert(is_int("-20000000000000000000"), "Arbitrary big int is int (may change)");

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
	static_assert(to_int<int>("0").unsignedValue.value_or(123) == 0, "0 becomes unsigned");
	static_assert(to_int<int>("1").unsignedValue.value_or(123) == 1, "Positive becomes unsigned");
	static_assert(to_int<int>("+1").unsignedValue.value_or(123) == 1, "Explicit positive becomes unsigned");
	static_assert(to_int<int>("-1").signedValue.value_or(123) == -1, "Negative becomes signed");
	static_assert(to_int<int>("001").unsignedValue.value_or(123) == 1, "Handles padding zeroes, positive");
	static_assert(to_int<int>("-001").signedValue.value_or(123) == -1, "Handles padding zeroes, negative");
	static_assert(to_int<int>("123").unsignedValue.value_or(0) == 123, "Arbitrary number");
	static_assert(to_int<int>( "2147483647").unsignedValue.value_or(0) == 2147483647, "Max int32 becomes uint32");
	static_assert(to_int<int>("-2147483648").signedValue.value_or(0) == (-2147483647 - 1), "Min int32 becomes int32");
	static_assert(to_int<int>("4294967295").unsignedValue.value_or(0) == 4294967295, "Max uint32 becomes uint32");
	static_assert(to_int<int64_t>("2147483647").unsignedValue.value_or(0) == 2147483647, "Max int32 becomes uint32 also with wider type");
	static_assert(to_int<int64_t>("-2147483648").signedValue.value_or(0) == (-2147483647 - 1), "Min int32 becomes int32 also with wider type");
	static_assert(to_int<int64_t>("4294967295").unsignedValue.value_or(0) == 4294967295, "Max uint32 becomes uint32 also with wider type");
	static_assert(to_int<int64_t>("9223372036854775807").unsignedValue.value_or(0) == 9223372036854775807, "Max int64 becomes uint64");
	static_assert(to_int<int64_t>("-9223372036854775808").signedValue.value_or(0) == (-9223372036854775807 - 1), "Min int64 becomes int64");
	static_assert(to_int<int64_t>("18446744073709551615").unsignedValue.value_or(0) == 18446744073709551615, "Max uint64 becomes uint64");

	expect(!to_int<int64_t>("20000000000000000000").has_value()) << "Arbitrary width, positive big int can't be converted";
	expect(!to_int<int64_t>("-20000000000000000000").has_value()) << "Arbitrary width, negative big int can't be converted";
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
