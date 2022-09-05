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
