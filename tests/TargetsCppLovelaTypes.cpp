#include "pch.h"
#include "../targets/cpp/source/lovela.h"

suite Variable = [] {
	"SetGetSize"_test = [] {
		lovela::variable<int> obj1;
		expect(nothrow([&] { obj1.set_size(1); }));
		expect(throws<std::out_of_range>([&] { obj1.set_size(2); }));
		expect(obj1.get_size() == 1);
	};
};
