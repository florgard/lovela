#include "pch.h"
#include "../targets/cpp/lovela-runtime/lovela.h"

auto f_RaisesError(lovela::context& context, const auto& in)
{
	static_cast<void>(context);
	auto& v1 = in; static_cast<void>(v1);
	if (v1) { throw lovela::error("error"); }
	return v1;
}

auto f_ErrorHandlerReset(lovela::context& context, const auto& in)
{
	static_cast<void>(context);
	auto& v1 = in; static_cast<void>(v1);
	return v1;
}

auto fb_WithTailErrorHandlerOnError(lovela::context& context, const auto& in)
{
	static_cast<void>(context);
	auto& v1 = in; static_cast<void>(v1);
	auto v2 = 200;
	auto v3 = f_RaisesError(context, v2);
	return v3;
}

auto f_WithTailErrorHandlerOnError(lovela::context& context, const auto& in)
{
	auto& i1 = in; static_cast<void>(i1);

	std::remove_reference_t<decltype(fb_WithTailErrorHandlerOnError(context, i1))> o1;

	try
	{
		o1 = fb_WithTailErrorHandlerOnError(context, i1);
	}
	catch (const lovela::error& error)
	{
		context.error = error;
		o1 = f_ErrorHandlerReset(context, i1);
	}

	return o1;
}

auto fb_WithTailErrorHandlerOnSuccess(lovela::context& context, const auto& in)
{
	static_cast<void>(context);
	auto& v1 = in; static_cast<void>(v1);
	auto v2 = 200;
	return v2;
}

auto f_WithTailErrorHandlerOnSuccess(lovela::context& context, const auto& in)
{
	auto& i1 = in; static_cast<void>(i1);

	std::remove_reference_t<decltype(fb_WithTailErrorHandlerOnSuccess(context, i1))> o1;

	try
	{
		o1 = fb_WithTailErrorHandlerOnSuccess(context, i1);
	}
	catch (const lovela::error& error)
	{
		context.error = error;
		o1 = f_ErrorHandlerReset(context, i1);
	}

	return o1;
}

auto fb1_WithMidErrorHandlerOnError(lovela::context& context, const auto& in)
{
	static_cast<void>(context);
	auto& v1 = in; static_cast<void>(v1);
	auto v2 = v1 + 10;
	auto v3 = f_RaisesError(context, v2);
	return v3;
}

auto fb2_WithMidErrorHandlerOnError(lovela::context& context, const auto& in)
{
	static_cast<void>(context);
	auto& v1 = in; static_cast<void>(v1);
	auto v2 = v1 + 1.23;
	return v2;
}

auto f_WithMidErrorHandlerOnError(lovela::context& context, const auto& in)
{
	auto& i1 = in; static_cast<void>(i1);

	std::remove_reference_t<decltype(fb1_WithMidErrorHandlerOnError(context, i1))> o1;

	try
	{
		o1 = fb1_WithMidErrorHandlerOnError(context, i1);
	}
	catch (const lovela::error& error)
	{
		context.error = error;
		o1 = f_ErrorHandlerReset(context, i1);
	}

	auto& i2 = o1;

	std::remove_reference_t<decltype(fb2_WithMidErrorHandlerOnError(context, i2))> o2;

	try
	{
		o2 = fb2_WithMidErrorHandlerOnError(context, i2);
	}
	catch (const lovela::error& error)
	{
		context.error = error;
		o2 = f_ErrorHandlerReset(context, i2);
	}

	return o2;
}

auto fb1_WithMidErrorHandlerOnSuccess(lovela::context& context, const auto& in)
{
	static_cast<void>(context);
	auto& v1 = in; static_cast<void>(v1);
	auto v2 = v1 + 10;
	return v2;
}

auto fb2_WithMidErrorHandlerOnSuccess(lovela::context& context, const auto& in)
{
	static_cast<void>(context);
	auto& v1 = in; static_cast<void>(v1);
	auto v2 = v1 + 1.23;
	return v2;
}

auto f_WithMidErrorHandlerOnSuccess(lovela::context& context, const auto& in)
{
	auto& i1 = in; static_cast<void>(i1);

	std::remove_reference_t<decltype(fb1_WithMidErrorHandlerOnSuccess(context, i1))> o1;

	try
	{
		o1 = fb1_WithMidErrorHandlerOnSuccess(context, i1);
	}
	catch (const lovela::error& error)
	{
		context.error = error;
		o1 = f_ErrorHandlerReset(context, i1);
	}

	auto& i2 = o1;

	std::remove_reference_t<decltype(fb2_WithMidErrorHandlerOnSuccess(context, i2))> o2;

	try
	{
		o2 = fb2_WithMidErrorHandlerOnSuccess(context, i2);
	}
	catch (const lovela::error& error)
	{
		context.error = error;
		o2 = f_ErrorHandlerReset(context, i2);
	}

	return o2;
}

suite ErrorHandling = [] {
	"RaisesError"_test = [] {
		lovela::context context;
		expect(throws<lovela::error>([&] { static_cast<void>(f_RaisesError(context, 100)); }));
	};

	"WithTailErrorHandlerOnError"_test = [] {
		lovela::context context;
		expect(f_WithTailErrorHandlerOnError(context, 100) == 100);
	};

	"WithTailErrorHandlerOnSuccess"_test = [] {
		lovela::context context;
		expect(f_WithTailErrorHandlerOnSuccess(context, 100) == 200);
	};

	"WithMidErrorHandlerOnError"_test = [] {
		lovela::context context;
		expect(f_WithMidErrorHandlerOnError(context, 100) == 101.23);
	};

	"WithMidErrorHandlerOnSuccess"_test = [] {
		lovela::context context;
		expect(f_WithMidErrorHandlerOnSuccess(context, 100) == 111.23);
	};
};
