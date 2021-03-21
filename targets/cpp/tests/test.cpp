#include "pch.h"
#include "test.h"

auto f_ReturnInput(lovela::context& context, const auto& in)
{
	auto& v1 = in; v1; context;
	return v1;
}

auto f_ReturnInputIncremented(lovela::context& context, const auto& in)
{
	auto& v1 = in; v1; context;
	auto v2 = v1 + 1;
	return v2;
}

#pragma warning(push)
#pragma warning(disable: 4702)
auto f_RaisesError(lovela::context& context, const auto& in)
{
	auto& v1 = in; v1; context;
	throw lovela::error("error");
	return v1;
}
#pragma warning(pop)

auto f_ErrorHandlerReset(lovela::context& context, const auto& in)
{
	auto& v1 = in; v1; context;
	return v1;
}

auto fb_WithTailErrorHandlerOnError(lovela::context& context, const auto& in)
{
	auto& v1 = in; v1; context;
	auto v2 = 200;
	auto v3 = f_RaisesError(context, v2);
	return v3;
}

auto f_WithTailErrorHandlerOnError(lovela::context& context, const auto& in)
{
	auto& i1 = in; i1;

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
	auto& v1 = in; v1; context;
	auto v2 = 200;
	return v2;
}

auto f_WithTailErrorHandlerOnSuccess(lovela::context& context, const auto& in)
{
	auto& i1 = in; i1;

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
	auto& v1 = in; v1; context;
	auto v2 = v1 + 10;
	auto v3 = f_RaisesError(context, v2);
	return v3;
}

auto fb2_WithMidErrorHandlerOnError(lovela::context& context, const auto& in)
{
	auto& v1 = in; v1; context;
	auto v2 = v1 + 1.23;
	return v2;
}

auto f_WithMidErrorHandlerOnError(lovela::context& context, const auto& in)
{
	auto& i1 = in; i1;

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
	auto& v1 = in; v1; context;
	auto v2 = v1 + 10;
	return v2;
}

auto fb2_WithMidErrorHandlerOnSuccess(lovela::context& context, const auto& in)
{
	auto& v1 = in; v1; context;
	auto v2 = v1 + 1.23;
	return v2;
}

auto f_WithMidErrorHandlerOnSuccess(lovela::context& context, const auto& in)
{
	auto& i1 = in; i1;

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

TEST(ReturnValueTest, ReturnInputIncremented) {
	lovela::context context;
	EXPECT_EQ(f_ReturnInputIncremented(context, 100), 101);
}

TEST(ReturnValueTest, RaisesError) {
	lovela::context context;
	EXPECT_THROW(f_RaisesError(context, 100), lovela::error);
}

TEST(ReturnValueTest, WithTailErrorHandlerOnError) {
	lovela::context context;
	EXPECT_EQ(f_WithTailErrorHandlerOnError(context, 100), 100);
}

TEST(ReturnValueTest, WithTailErrorHandlerOnSuccess) {
	lovela::context context;
	EXPECT_EQ(f_WithTailErrorHandlerOnSuccess(context, 100), 200);
}

TEST(ReturnValueTest, WithMidErrorHandlerOnError) {
	lovela::context context;
	EXPECT_EQ(f_WithMidErrorHandlerOnError(context, 100), 101.23);
}

TEST(ReturnValueTest, WithMidErrorHandlerOnSuccess) {
	lovela::context context;
	EXPECT_EQ(f_WithMidErrorHandlerOnSuccess(context, 100), 111.23);
}

namespace lovela
{
	int main(lovela::context& context)
	{
		context;
		None in;
		auto& v1 = in; v1;
		const auto v2 = 0;
		return v2;
	}
}

int MainTest(int argc, char* argv[])
{
	lovela::context context{ .parameters{argv + 1, argv + argc} };
	return lovela::main(context);
}

TEST(MainTest, Trivial) {
	char arg1[10];
	strcpy_s(arg1, "program");
	char* argv[]{ arg1 };
	EXPECT_EQ(MainTest(1, argv), 0);
}
