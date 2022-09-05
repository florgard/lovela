#include "pch.h"
#include "test.h"

#pragma warning(push)
#pragma warning(disable:4834)



#pragma warning(pop) // 4834



auto f_ReturnInput(lovela::context& context, const auto& in)
{
	context;
	auto& v1 = in; static_cast<void>(v1);
	return v1;
}

auto f_ReturnInputIncremented(lovela::context& context, const auto& in)
{
	context;
	auto& v1 = in; static_cast<void>(v1);
	auto v2 = v1 + 1;
	return v2;
}

#pragma warning(push)
#pragma warning(disable: 4702)
auto f_RaisesError(lovela::context& context, const auto& in)
{
	context;
	auto& v1 = in; static_cast<void>(v1);
	throw lovela::error("error");
	return v1;
}
#pragma warning(pop)

auto f_ErrorHandlerReset(lovela::context& context, const auto& in)
{
	context;
	auto& v1 = in; static_cast<void>(v1);
	return v1;
}

auto fb_WithTailErrorHandlerOnError(lovela::context& context, const auto& in)
{
	context;
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
	context;
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
	context;
	auto& v1 = in; static_cast<void>(v1);
	auto v2 = v1 + 10;
	auto v3 = f_RaisesError(context, v2);
	return v3;
}

auto fb2_WithMidErrorHandlerOnError(lovela::context& context, const auto& in)
{
	context;
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
	context;
	auto& v1 = in; static_cast<void>(v1);
	auto v2 = v1 + 10;
	return v2;
}

auto fb2_WithMidErrorHandlerOnSuccess(lovela::context& context, const auto& in)
{
	context;
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

TEST(MainTest, Trivial) {
	char arg0[10] = "";
	char* argv[]{ arg0, nullptr };
	int argc = sizeof(argv) / sizeof(argv[0]) - 1;
	EXPECT_EQ(MainTest(argc, argv), 0);
}

TEST(MainTest, Context) {
	char arg0[10] = "program";
	char arg1[10] = "param";
	char* argv[]{ arg0, arg1, nullptr };
	int argc = sizeof(argv) / sizeof(argv[0]) - 1;
	lovela::context context{ .parameters{argv + 1, argv + argc} };
	EXPECT_EQ(context.select<4>().size(), 1);
	EXPECT_EQ(context.select<4>()[0], "param");
}
