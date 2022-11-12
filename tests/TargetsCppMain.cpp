#include "pch.h"
#include "../targets/cpp/lovela-runtime/lovela.h"

#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp"
using namespace boost::ut;

lovela::None lovela::main(lovela::context& context, lovela::None in)
{
	static_cast<void>(context);
	auto& v1 = in; static_cast<void>(v1);
	return {};
}

int MainTest(int argc, char* argv[])
{
	lovela::context context{ .parameters{argv + 1, argv + argc} };
	lovela::None in;
	lovela::main(context, in);
	return context.error.code;
}

suite Main = [] {
	"Trivial"_test = [] {
		char arg0[10] = "";
		char* argv[]{ arg0, nullptr };
		int argc = sizeof(argv) / sizeof(argv[0]) - 1;
		expect(MainTest(argc, argv) == 0);
	};

	"Context"_test = [] {
		char arg0[10] = "program";
		char arg1[10] = "param";
		char* argv[]{ arg0, arg1, nullptr };
		int argc = sizeof(argv) / sizeof(argv[0]) - 1;
		lovela::context context{ .parameters{argv + 1, argv + argc} };
		expect(context.select<4>().size() == 1);
		expect(context.select<4>()[0] == "param");
	};
};
