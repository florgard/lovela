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

auto fb_WithErrorHandlerError(lovela::context& context, const auto& in)
{
	auto& v1 = in; v1; context;
	auto v2 = 456;
	auto v3 = f_RaisesError(context, v2);
	return v3;
}

auto f_WithErrorHandlerError(lovela::context& context, const auto& in)
{
	decltype(fb_WithErrorHandlerError(context, in)) out;

	try
	{
		out = fb_WithErrorHandlerError(context, in);
	}
	catch (const lovela::error& error)
	{
		context.error = error;
		out = f_ErrorHandlerReset(context, in);
	}

	return out;
}

auto fb_WithErrorHandlerNoError(lovela::context& context, const auto& in)
{
	auto& v1 = in; v1; context;
	auto v2 = 456;
	return v2;
}

auto f_WithErrorHandlerNoError(lovela::context& context, const auto& in)
{
	decltype(fb_WithErrorHandlerNoError(context, in)) out;

	try
	{
		out = fb_WithErrorHandlerNoError(context, in);
	}
	catch (const lovela::error& error)
	{
		context.error = error;
		out = f_ErrorHandlerReset(context, in);
	}

	return out;
}

TEST(ReturnValueTest, ReturnInput) {
	lovela::context context;
	EXPECT_EQ(f_ReturnInput(context, 123), 123);
}

TEST(ReturnValueTest, ReturnInputIncremented) {
	lovela::context context;
	EXPECT_EQ(f_ReturnInputIncremented(context, 123), 124);
}

TEST(ReturnValueTest, RaisesError) {
	lovela::context context;
	EXPECT_THROW(f_RaisesError(context, 123), lovela::error);
}

TEST(ReturnValueTest, WithErrorHandlerError) {
	lovela::context context;
	EXPECT_EQ(f_WithErrorHandlerError(context, 123), 123);
}

TEST(ReturnValueTest, WithErrorHandlerNoError) {
	lovela::context context;
	EXPECT_EQ(f_WithErrorHandlerNoError(context, 123), 456);
}
