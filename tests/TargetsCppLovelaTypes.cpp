#include "pch.h"
#include "../targets/cpp/source/lovela.h"

suite Variable = [] {
	"SetGetSize"_test = [] {
		lovela::variable<int> obj1;
		expect(nothrow([&] { obj1.set_size(1); }));
		expect(throws<std::out_of_range>([&] { obj1.set_size(2); }));
		expect(obj1.get_size() == 1);
	};

	"Types"_test = [] {
		lovela::variable<std::string> obj1;
		lovela::variable<int> obj2;

		std::string r1;
		expect(nothrow([&] { obj1.set_item<1>("aaa"); }));
		expect(nothrow([&] { obj1.get_item<1>(r1); }));
		expect(r1 == "aaa");

		int r2{};
		expect(nothrow([&] { obj2.set_item<1>(123); }));
		expect(nothrow([&] { obj2.get_item<1>(r2); }));
		expect(r2 == 123);
	};

	"SetGetAddRange"_test = [] {
		lovela::variable<std::string> obj1;

		std::string r1;
		expect(nothrow([&] { obj1.set_item<1>("aaa"); }));
		expect(obj1.get_item<1>() == "aaa");
		expect(nothrow([&] { obj1.get_item<1>(r1); }));
		expect(r1 == "aaa");
		expect(nothrow([&] { obj1.get_item(1, r1); }));
		expect(r1 == "aaa");
		expect(nothrow([&] { obj1.get_item(u8"1", r1); }));
		expect(r1 == "aaa");

		expect(nothrow([&] { obj1.set_item(1, "bbb"); }));
		expect(obj1.get_item<1>() == "bbb");
		expect(nothrow([&] { obj1.get_item<1>(r1); }));
		expect(r1 == "bbb");
		expect(nothrow([&] { obj1.get_item(1, r1); }));
		expect(r1 == "bbb");
		expect(nothrow([&] { obj1.get_item(u8"1", r1); }));
		expect(r1 == "bbb");

		expect(nothrow([&] { obj1.set_item(u8"1", "ccc"); }));
		expect(obj1.get_item<1>() == "ccc");
		expect(nothrow([&] { obj1.get_item<1>(r1); }));
		expect(r1 == "ccc");
		expect(nothrow([&] { obj1.get_item(1, r1); }));
		expect(r1 == "ccc");
		expect(nothrow([&] { obj1.get_item(u8"1", r1); }));
		expect(r1 == "ccc");

		expect(throws<std::out_of_range>([&] { obj1.get_item(0, r1); }));
		expect(throws<std::out_of_range>([&] { obj1.set_item(2, "cde"); }));

		expect(throws<std::out_of_range>([&] { obj1.add_item("cde"); }));
		expect(obj1.get_size() == 1);
	};

	"GetIndex"_test = [] {
		lovela::variable<std::string> obj1;

		expect(obj1.get_index(u8"1") == 1);
		expect(throws<std::invalid_argument>([&] { static_cast<void>(obj1.get_index(u8"")); }));
		expect(throws<std::invalid_argument>([&] { static_cast<void>(obj1.get_index(u8"null")); }));
		expect(throws<std::out_of_range>([&] { static_cast<void>(obj1.get_index(u8"0")); }));
		expect(throws<std::out_of_range>([&] { static_cast<void>(obj1.get_index(u8"2")); }));
	};
};

suite IndexRebase = [] {
	"TestRebaseAndRange"_test = [] {
		expect(lovela::detail::rebase(1, 10) == 0);
		expect(lovela::detail::rebase(10, 10) == 9);
		expect(throws<std::out_of_range>([&] { static_cast<void>(lovela::detail::rebase(0, 10)); }));
		expect(throws<std::out_of_range>([&] { static_cast<void>(lovela::detail::rebase(11, 10)); }));
	};
};

suite FixedTuple = [] {
	"InitAndRange"_test = [] {
		lovela::fixed_tuple<std::tuple<int, double, std::string>> obj1;
		expect(nothrow([&] { obj1.set_size(3); }));
		expect(throws<std::out_of_range>([&] { obj1.set_size(20); }));
		expect(obj1.get_size() == 3);
		expect(throws<std::out_of_range>([&] { obj1.add_item(123); }));
	};

	"SetGet"_test = [] {
		lovela::fixed_tuple<std::tuple<int, double, std::string>> obj1;
		expect(nothrow([&] { obj1.set_item<1>(123); }));
		expect(nothrow([&] { obj1.set_item<2>(456.789); }));
		expect(nothrow([&] { obj1.set_item<3>(std::string("abc")); }));

		int r1{};
		expect(obj1.get_item<1>() == 123);
		expect(nothrow([&] { static_cast<void>(obj1.get_item<1>(r1)); }));
		expect(r1 == 123);
		expect(obj1.get_item<int>(1) == 123);
		expect(nothrow([&] { static_cast<void>(obj1.get_item(1, r1)); }));
		expect(r1 == 123);
		expect(obj1.get_item<int>(u8"1") == 123);
		expect(nothrow([&] { static_cast<void>(obj1.get_item(u8"1", r1)); }));
		expect(r1 == 123);

		double r2{};
		expect(obj1.get_item<2>() == 456.789);
		expect(nothrow([&] { static_cast<void>(obj1.get_item<2>(r2)); }));
		expect(r2 == 456.789);
		expect(obj1.get_item<double>(2) == 456.789);
		expect(nothrow([&] { static_cast<void>(obj1.get_item(2, r2)); }));
		expect(r2 == 456.789);
		expect(obj1.get_item<double>(u8"2") == 456.789);
		expect(nothrow([&] { static_cast<void>(obj1.get_item(u8"2", r2)); }));
		expect(r2 == 456.789);

		std::string r3{};
		expect(obj1.get_item<3>() == "abc");
		expect(nothrow([&] { static_cast<void>(obj1.get_item<3>(r3)); }));
		expect(r3 == "abc");
		expect(obj1.get_item<std::string>(3) == "abc");
		expect(nothrow([&] { static_cast<void>(obj1.get_item(3, r3)); }));
		expect(r3 == "abc");
		expect(obj1.get_item<std::string>(u8"3") == "abc");
		expect(nothrow([&] { static_cast<void>(obj1.get_item(u8"3", r3)); }));
		expect(r3 == "abc");
	};

	"Range"_test = [] {
		lovela::fixed_tuple<std::tuple<int, double, std::string>> obj1;
		int r1{};
		expect(throws<std::invalid_argument>([&] { static_cast<void>(obj1.get_item(3, r1)); }));
		expect(throws<std::out_of_range>([&] { static_cast<void>(obj1.get_item(0, r1)); }));
		expect(throws<std::out_of_range>([&] { static_cast<void>(obj1.get_item(4, r1)); }));
		expect(throws<std::invalid_argument>([&] { static_cast<void>(obj1.get_item(u8"", r1)); }));
		expect(throws<std::invalid_argument>([&] { static_cast<void>(obj1.get_item(u8"null", r1)); }));
		expect(throws<std::out_of_range>([&] { static_cast<void>(obj1.get_item(u8"0", r1)); }));
		expect(throws<std::out_of_range>([&] { static_cast<void>(obj1.get_item(u8"4", r1)); }));
	};

	"GetIndex"_test = [] {
		lovela::fixed_tuple<std::tuple<int, double, std::string>> obj1;
		expect(obj1.get_index(u8"1") == 1);
		expect(throws<std::invalid_argument>([&] { static_cast<void>(obj1.get_index(u8"")); }));
		expect(throws<std::invalid_argument>([&] { static_cast<void>(obj1.get_index(u8"null")); }));
		expect(throws<std::out_of_range>([&] { static_cast<void>(obj1.get_index(u8"0")); }));
		expect(throws<std::out_of_range>([&] { static_cast<void>(obj1.get_index(u8"4")); }));
	};
};
