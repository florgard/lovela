#include "pch.h"
#include "../targets/cpp/lovela-runtime/lovela.h"

auto f_ReturnInput(lovela::context& context, const auto& in)
{
	static_cast<void>(context);
	auto& v1 = in; static_cast<void>(v1);
	return v1;
}

auto f_ReturnInputIncremented(lovela::context& context, const auto& in)
{
	static_cast<void>(context);
	auto& v1 = in; static_cast<void>(v1);
	auto v2 = v1 + 1;
	return v2;
}

suite FunctionCalls = [] {
	"ReturnInput"_test = [] {
		lovela::context context;
		expect(f_ReturnInput(context, 100) == 100);
	};

	"ReturnInputIncremented"_test = [] {
		lovela::context context;
		expect(f_ReturnInputIncremented(context, 100) == 101);
	};
};
