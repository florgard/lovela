#include "pch.h"
#include "test.h"

auto f_success(lovela::context& context, const auto& in)
{
	auto& v1 = in; v1; context;
	return v1;
}

#pragma warning(push)
#pragma warning(disable: 4702)
auto f_error(lovela::context& context, const auto& in)
{
	auto& v1 = in; v1; context;
	throw lovela::error("error");
	return v1;
}
#pragma warning(pop)

auto f_error_handler(lovela::context& context, const auto& in)
{
	auto& v1 = in; v1; context;
	auto v2 = 0;
	return v2;
}

auto f_inner_error(lovela::context& context, const auto& in)
{
	auto& v1 = in; v1; context;

	try
	{
		auto v2 = f_error(context, v1);
		return v2;
	}
	catch (const lovela::error& error)
	{
		context.error = error;
		auto v3 = f_error_handler(context, in);
		return v3;
	}
	catch (const std::exception&)
	{
		throw;
	}
}

TEST(ReturnValueTest, Success) {
	lovela::context context;
	EXPECT_EQ(f_success(context, 123), 123);
}

TEST(ReturnValueTest, UnhandledError) {
	lovela::context context;
	EXPECT_THROW(f_error(context, 123), lovela::error);
}

TEST(ReturnValueTest, HandledError) {
	lovela::context context;
	EXPECT_EQ(f_inner_error(context, 123), 0);
}
