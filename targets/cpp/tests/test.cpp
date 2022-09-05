#include "pch.h"
#include "test.h"

#pragma warning(push)
#pragma warning(disable:4834)

TEST(DynamicArray, SetGetSize) {
	lovela::dynamic_array<std::string> obj1;
	EXPECT_NO_THROW(obj1.set_size(10));
	EXPECT_NO_THROW(obj1.set_size(20));
	EXPECT_EQ(obj1.get_size(), 20);
}

TEST(DynamicArray, Types) {
	lovela::dynamic_array<std::string> obj1;
	lovela::dynamic_array<int> obj2;

	EXPECT_NO_THROW(obj1.set_size(20));
	EXPECT_NO_THROW(obj2.set_size(20));

	std::string r1;
	EXPECT_NO_THROW(obj1.set_item<10>("aaa"));
	EXPECT_NO_THROW(obj1.get_item<10>(r1));
	EXPECT_EQ(r1, "aaa");

	int r2{};
	EXPECT_NO_THROW(obj2.set_item<10>(123));
	EXPECT_NO_THROW(obj2.get_item<10>(r2));
	EXPECT_EQ(r2, 123);
}

TEST(DynamicArray, SetGetAddRange) {
	lovela::dynamic_array<std::string> obj1;

	EXPECT_NO_THROW(obj1.set_size(20));

	std::string r1;
	EXPECT_NO_THROW(obj1.set_item<1>("aaa"));
	EXPECT_EQ(obj1.get_item<1>(), "aaa");
	EXPECT_NO_THROW(obj1.get_item<1>(r1));
	EXPECT_EQ(r1, "aaa");
	EXPECT_NO_THROW(obj1.get_item(1, r1));
	EXPECT_EQ(r1, "aaa");
	EXPECT_NO_THROW(obj1.get_item(u8"1", r1));
	EXPECT_EQ(r1, "aaa");

	EXPECT_NO_THROW(obj1.set_item(1, "bbb"));
	EXPECT_EQ(obj1.get_item<1>(), "bbb");
	EXPECT_NO_THROW(obj1.get_item<1>(r1));
	EXPECT_EQ(r1, "bbb");
	EXPECT_NO_THROW(obj1.get_item(1, r1));
	EXPECT_EQ(r1, "bbb");
	EXPECT_NO_THROW(obj1.get_item(u8"1", r1));
	EXPECT_EQ(r1, "bbb");

	EXPECT_NO_THROW(obj1.set_item(u8"1", "ccc"));
	EXPECT_EQ(obj1.get_item<1>(), "ccc");
	EXPECT_NO_THROW(obj1.get_item<1>(r1));
	EXPECT_EQ(r1, "ccc");
	EXPECT_NO_THROW(obj1.get_item(1, r1));
	EXPECT_EQ(r1, "ccc");
	EXPECT_NO_THROW(obj1.get_item(u8"1", r1));
	EXPECT_EQ(r1, "ccc");

	EXPECT_NO_THROW(obj1.set_item(20, "bcd"));
	EXPECT_NO_THROW(obj1.get_item(20, r1));
	EXPECT_EQ(r1, "bcd");

	EXPECT_THROW(obj1.get_item(0, r1), std::out_of_range);
	EXPECT_THROW(obj1.set_item(21, "cde"), std::out_of_range);

	EXPECT_NO_THROW(obj1.add_item("def"));
	EXPECT_NO_THROW(obj1.get_item(21, r1));
	EXPECT_EQ(r1, "def");
	EXPECT_EQ(obj1.get_size(), 21);
}

TEST(DynamicArray, GetIndex) {
	lovela::dynamic_array<std::string> obj1;

	EXPECT_NO_THROW(obj1.set_size(20));

	EXPECT_EQ(obj1.get_index(u8"1"), 1);
	EXPECT_THROW(obj1.get_index(u8""), std::invalid_argument);
	EXPECT_THROW(obj1.get_index(u8"null"), std::invalid_argument);
	EXPECT_THROW(obj1.get_index(u8"0"), std::out_of_range);
	EXPECT_THROW(obj1.get_index(u8"21"), std::out_of_range);
}

TEST(FixedArray, SetGetSize) {
	lovela::fixed_array<int, 10> obj1;
	EXPECT_NO_THROW(obj1.set_size(10));
	EXPECT_THROW(obj1.set_size(20), std::out_of_range);
	EXPECT_EQ(obj1.get_size(), 10);
}

TEST(FixedArray, Types) {
	lovela::fixed_array<std::string, 20> obj1;
	lovela::fixed_array<int, 20> obj2;

	EXPECT_NO_THROW(obj1.set_size(20));
	EXPECT_NO_THROW(obj2.set_size(20));

	std::string r1;
	EXPECT_NO_THROW(obj1.set_item<10>("aaa"));
	EXPECT_NO_THROW(obj1.get_item<10>(r1));
	EXPECT_EQ(r1, "aaa");

	int r2{};
	EXPECT_NO_THROW(obj2.set_item<10>(123));
	EXPECT_NO_THROW(obj2.get_item<10>(r2));
	EXPECT_EQ(r2, 123);
}

TEST(FixedArray, SetGetAddRange) {
	lovela::fixed_array<std::string, 20> obj1;

	EXPECT_NO_THROW(obj1.set_size(20));

	std::string r1;
	EXPECT_NO_THROW(obj1.set_item<1>("aaa"));
	EXPECT_EQ(obj1.get_item<1>(), "aaa");
	EXPECT_NO_THROW(obj1.get_item<1>(r1));
	EXPECT_EQ(r1, "aaa");
	EXPECT_NO_THROW(obj1.get_item(1, r1));
	EXPECT_EQ(r1, "aaa");
	EXPECT_NO_THROW(obj1.get_item(u8"1", r1));
	EXPECT_EQ(r1, "aaa");

	EXPECT_NO_THROW(obj1.set_item(1, "bbb"));
	EXPECT_EQ(obj1.get_item<1>(), "bbb");
	EXPECT_NO_THROW(obj1.get_item<1>(r1));
	EXPECT_EQ(r1, "bbb");
	EXPECT_NO_THROW(obj1.get_item(1, r1));
	EXPECT_EQ(r1, "bbb");
	EXPECT_NO_THROW(obj1.get_item(u8"1", r1));
	EXPECT_EQ(r1, "bbb");

	EXPECT_NO_THROW(obj1.set_item(u8"1", "ccc"));
	EXPECT_EQ(obj1.get_item<1>(), "ccc");
	EXPECT_NO_THROW(obj1.get_item<1>(r1));
	EXPECT_EQ(r1, "ccc");
	EXPECT_NO_THROW(obj1.get_item(1, r1));
	EXPECT_EQ(r1, "ccc");
	EXPECT_NO_THROW(obj1.get_item(u8"1", r1));
	EXPECT_EQ(r1, "ccc");

	EXPECT_NO_THROW(obj1.set_item(20, "bcd"));
	EXPECT_NO_THROW(obj1.get_item(20, r1));
	EXPECT_EQ(r1, "bcd");

	EXPECT_THROW(obj1.get_item(0, r1), std::out_of_range);
	EXPECT_THROW(obj1.set_item(21, "cde"), std::out_of_range);

	EXPECT_THROW(obj1.add_item("cde"), std::out_of_range);
	EXPECT_EQ(obj1.get_size(), 20);
}

TEST(FixedArray, GetIndex) {
	lovela::fixed_array<std::string, 20> obj1;

	EXPECT_NO_THROW(obj1.set_size(20));

	EXPECT_EQ(obj1.get_index(u8"1"), 1);
	EXPECT_THROW(obj1.get_index(u8""), std::invalid_argument);
	EXPECT_THROW(obj1.get_index(u8"null"), std::invalid_argument);
	EXPECT_THROW(obj1.get_index(u8"0"), std::out_of_range);
	EXPECT_THROW(obj1.get_index(u8"21"), std::out_of_range);
}

#pragma warning(pop) // 4834

TEST(Convert, ArrayToTuple) {
	std::array<int, 3> obj1{ 5, 4, 10 };
	auto obj2 = lovela::detail::as_tuple(std::move(obj1));
	EXPECT_EQ(std::get<1>(obj2), 4);
}

TEST(Convert, ToFixedTuple) {
	int r1{};
	lovela::to_fixed_tuple(6).get_item<1>(r1);
	EXPECT_EQ(r1, 6);
	lovela::to_fixed_tuple(6, 23).get_item<2>(r1);
	EXPECT_EQ(r1, 23);

	double r2{};
	EXPECT_NO_THROW(lovela::to_fixed_tuple(6, 23.5).get_item(2, r2));
	EXPECT_EQ(r2, 23.5);

	auto t = std::make_tuple(12.3, 45);
	EXPECT_NO_THROW(lovela::to_fixed_tuple(std::move(t)).get_item(2, r1));
	EXPECT_EQ(r1, 45);

	auto ft = lovela::to_fixed_tuple(6, 1.5);
	EXPECT_NO_THROW(lovela::to_fixed_tuple(std::move(ft)).get_item(2, r2));
	EXPECT_EQ(r2, 1.5);

	lovela::named_tuple<std::tuple<int, double, std::string>, l_tuple_names_1> nt{ {10, 5.25, "Boots"} };
	EXPECT_NO_THROW(lovela::to_fixed_tuple(std::move(nt)).get_item(2, r2));
	EXPECT_EQ(r2, 5.25);
}

TEST(FixedTuple, Combine) {
	using v1_t = lovela::fixed_tuple<std::tuple<int, double, std::string>>;
	v1_t v1{ {10, 5.25, "Boots"} };
	using v2_t = lovela::fixed_tuple<std::tuple<double, double, double>>;
	v2_t v2{ {7.75, 1.25, 0.1} };

	using v3_t = lovela::fixed_tuple<std::tuple<v2_t::item_type<1>, v1_t::item_type<3>, v1_t::item_type<1>>>;
	v3_t v3{ { v2.get_item<1>(), v1.get_item<3>(), v1.get_item<1>()} };

	EXPECT_EQ(v3.get_item<1>(), 7.75);
	EXPECT_EQ(v3.get_item<2>(), "Boots");
	EXPECT_EQ(v3.get_item<3>(), 10);
}

TEST(Streams, SimpleOut) {
	std::wostringstream r1;
	auto* buf = std::wcout.rdbuf(r1.rdbuf());
	lovela::streams streams;
	streams.select(2).write("abc");
	EXPECT_STREQ(r1.str().c_str(), L"abc");
	std::wcout.rdbuf(buf);
}

TEST(Streams, Utf8Out) {
	std::wostringstream r1;
	auto* buf = std::wcout.rdbuf(r1.rdbuf());
	lovela::streams streams;
	streams.select(2).write("100\xE2\x82\xAC");
	EXPECT_STREQ(r1.str().c_str(), L"100€");
	std::wcout.rdbuf(buf);
}

TEST(LovelaDataStructures, Error) {
	lovela::error e1("msg");
	lovela::error e2("msg", 2);
	lovela::error e3("msg", "type", 3);
	std::runtime_error rte("rte");
	lovela::error e4 = lovela::error::make_error(rte, 4);

	EXPECT_EQ(e1.code, 0);
	EXPECT_EQ(e1.message, "msg");
	EXPECT_EQ(e2.code, 2);
	EXPECT_EQ(e3.type, "type");
	EXPECT_EQ(e3.code, 3);
	EXPECT_EQ(e4.message, "rte");
	EXPECT_EQ(e4.type, typeid(std::runtime_error).name());
	EXPECT_EQ(e4.code, 4);
	EXPECT_STREQ(e4.inner.what(), "rte");
	EXPECT_EQ(e4.select<1>(), "rte");
	EXPECT_EQ(e4.select<2>(), typeid(std::runtime_error).name());
	EXPECT_EQ(e4.select<3>(), 4);
	EXPECT_STREQ(e4.select<4>().what(), "rte");
}

TEST(LovelaDataStructures, Context) {
	lovela::context context{ .error{"msg"}, .parameters{"param"} };
	EXPECT_EQ(context.select<2>().select<1>(), "msg");
	EXPECT_EQ(context.select<4>().front(), "param");
}

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
