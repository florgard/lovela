#include "pch.h"
#include "../targets/cpp/source/lovela-types.h"

suite FixedArray = [] {
	"SetGetSize"_test = [] {
		lovela::fixed_array<int, 10> obj1;
		expect(nothrow([&] { obj1.set_size(10); }));
		expect(throws<std::out_of_range>([&] { static_cast<void>(obj1.set_size(20)); }));
		expect(obj1.get_size() == 10);
	};

	"Types"_test = [] {
		lovela::fixed_array<std::string, 20> obj1;
		lovela::fixed_array<int, 20> obj2;

		expect(nothrow([&] { obj1.set_size(20); }));
		expect(nothrow([&] { obj2.set_size(20); }));

		std::string r1;
		expect(nothrow([&] { obj1.set_item<10>("aaa"); }));
		expect(nothrow([&] { obj1.get_item<10>(r1); }));
		expect(r1 == "aaa");

		int r2{};
		expect(nothrow([&] { obj2.set_item<10>(123); }));
		expect(nothrow([&] { obj2.get_item<10>(r2); }));
		expect(r2 == 123);
	};

	"SetGetAddRange"_test = [] {
		lovela::fixed_array<std::string, 20> obj1;

		expect(nothrow([&] { obj1.set_size(20); }));

		std::string r1;
		expect(nothrow([&] { obj1.set_item<1>("aaa"); }));
		expect(obj1.get_item<1>() == "aaa");
		expect(nothrow([&] { static_cast<void>(obj1.get_item<1>(r1)); }));
		expect(r1 == "aaa");
		expect(nothrow([&] { static_cast<void>(obj1.get_item(1, r1)); }));
		expect(r1 == "aaa");
		expect(nothrow([&] { static_cast<void>(obj1.get_item(u8"1", r1)); }));
		expect(r1 == "aaa");

		expect(nothrow([&] { obj1.set_item(1, "bbb"); }));
		expect(obj1.get_item<1>() == "bbb");
		expect(nothrow([&] { static_cast<void>(obj1.get_item<1>(r1)); }));
		expect(r1 == "bbb");
		expect(nothrow([&] { static_cast<void>(obj1.get_item(1, r1)); }));
		expect(r1 == "bbb");
		expect(nothrow([&] { static_cast<void>(obj1.get_item(u8"1", r1)); }));
		expect(r1 == "bbb");

		expect(nothrow([&] { obj1.set_item(u8"1", "ccc"); }));
		expect(obj1.get_item<1>() == "ccc");
		expect(nothrow([&] { static_cast<void>(obj1.get_item<1>(r1)); }));
		expect(r1 == "ccc");
		expect(nothrow([&] { static_cast<void>(obj1.get_item(1, r1)); }));
		expect(r1 == "ccc");
		expect(nothrow([&] { static_cast<void>(obj1.get_item(u8"1", r1)); }));
		expect(r1 == "ccc");

		expect(nothrow([&] { obj1.set_item(20, "bcd"); }));
		expect(nothrow([&] { static_cast<void>(obj1.get_item(20, r1)); }));
		expect(r1 == "bcd");

		expect(throws<std::out_of_range>([&] { static_cast<void>(obj1.get_item(0, r1)); }));
		expect(throws<std::out_of_range>([&] { static_cast<void>(obj1.set_item(21, "cde")); }));

		expect(throws<std::out_of_range>([&] { static_cast<void>(obj1.add_item("cde")); }));
		expect(obj1.get_size() == 20);
	};

	"GetIndex"_test = [] {
		lovela::fixed_array<std::string, 20> obj1;

		expect(nothrow([&] { obj1.set_size(20); }));

		expect(obj1.get_index(u8"1") == 1);
		expect(throws<std::invalid_argument>([&] { static_cast<void>(obj1.get_index(u8"")); }));
		expect(throws<std::invalid_argument>([&] { static_cast<void>(obj1.get_index(u8"null")); }));
		expect(throws<std::out_of_range>([&] { static_cast<void>(obj1.get_index(u8"0")); }));
		expect(throws<std::out_of_range>([&] { static_cast<void>(obj1.get_index(u8"21")); }));
	};
};

suite DynamicArray = [] {
	"SetGetSize"_test = [] {
		lovela::dynamic_array<std::string> obj1;
		expect(nothrow([&] { obj1.set_size(10); }));
		expect(nothrow([&] { obj1.set_size(20); }));
		expect(obj1.get_size() == 20);
	};

	"Types"_test = [] {
		lovela::dynamic_array<std::string> obj1;
		lovela::dynamic_array<int> obj2;

		expect(nothrow([&] { obj1.set_size(20); }));
		expect(nothrow([&] { obj2.set_size(20); }));

		std::string r1;
		expect(nothrow([&] { obj1.set_item<10>("aaa"); }));
		expect(nothrow([&] { static_cast<void>(obj1.get_item<10>(r1)); }));
		expect(r1 == "aaa");

		int r2{};
		expect(nothrow([&] { obj2.set_item<10>(123); }));
		expect(nothrow([&] { static_cast<void>(obj2.get_item<10>(r2)); }));
		expect(r2 == 123);
	};

	"SetGetAddRange"_test = [] {
		lovela::dynamic_array<std::string> obj1;

		expect(nothrow([&] { obj1.set_size(20); }));

		std::string r1;
		expect(nothrow([&] { obj1.set_item<1>("aaa"); }));
		expect(obj1.get_item<1>() == "aaa");
		expect(nothrow([&] { static_cast<void>(obj1.get_item<1>(r1)); }));
		expect(r1 == "aaa");
		expect(nothrow([&] { static_cast<void>(obj1.get_item(1, r1)); }));
		expect(r1 == "aaa");
		expect(nothrow([&] { static_cast<void>(obj1.get_item(u8"1", r1)); }));
		expect(r1 == "aaa");

		expect(nothrow([&] { obj1.set_item(1, "bbb"); }));
		expect(obj1.get_item<1>() == "bbb");
		expect(nothrow([&] { static_cast<void>(obj1.get_item<1>(r1)); }));
		expect(r1 == "bbb");
		expect(nothrow([&] { static_cast<void>(obj1.get_item(1, r1)); }));
		expect(r1 == "bbb");
		expect(nothrow([&] { static_cast<void>(obj1.get_item(u8"1", r1)); }));
		expect(r1 == "bbb");

		expect(nothrow([&] { obj1.set_item(u8"1", "ccc"); }));
		expect(obj1.get_item<1>() == "ccc");
		expect(nothrow([&] { static_cast<void>(obj1.get_item<1>(r1)); }));
		expect(r1 == "ccc");
		expect(nothrow([&] { static_cast<void>(obj1.get_item(1, r1)); }));
		expect(r1 == "ccc");
		expect(nothrow([&] { static_cast<void>(obj1.get_item(u8"1", r1)); }));
		expect(r1 == "ccc");

		expect(nothrow([&] { obj1.set_item(20, "bcd"); }));
		expect(nothrow([&] { static_cast<void>(obj1.get_item(20, r1)); }));
		expect(r1 == "bcd");

		expect(throws<std::out_of_range>([&] { static_cast<void>(obj1.get_item(0, r1)); }));
		expect(throws<std::out_of_range>([&] { obj1.set_item(21, "cde"); }));

		expect(nothrow([&] { obj1.add_item("def"); }));
		expect(nothrow([&] { static_cast<void>(obj1.get_item(21, r1)); }));
		expect(r1 == "def");
		expect(obj1.get_size() == 21);
	};

	"GetIndex"_test = [] {
		lovela::dynamic_array<std::string> obj1;

		expect(nothrow([&] { obj1.set_size(20); }));

		expect(obj1.get_index(u8"1") == 1);
		expect(throws<std::invalid_argument>([&] { static_cast<void>(obj1.get_index(u8"")); }));
		expect(throws<std::invalid_argument>([&] { static_cast<void>(obj1.get_index(u8"null")); }));
		expect(throws<std::out_of_range>([&] { static_cast<void>(obj1.get_index(u8"0")); }));
		expect(throws<std::out_of_range>([&] { static_cast<void>(obj1.get_index(u8"21")); }));
	};
};
