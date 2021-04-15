#include "pch.h"
#include "test.h"

struct l_tuple_names_1
{
	static constexpr auto values = std::array<std::u8string_view, 3>{ u8"Pcs", u8"Price", u8"Name" };
};

struct l_tuple_names_2
{
	static constexpr auto values = std::array<std::u8string_view, 3>{ u8"Price", u8"Tax", u8"Discount" };
};

struct l_tuple_names_12
{
	static constexpr auto values = lovela::detail::array_cat(l_tuple_names_1::values, l_tuple_names_2::values);
};

TEST(NamedTuple, Concatenate) {
	lovela::named_tuple<l_tuple_names_1, std::tuple<int, double, std::string>> obj1{ {{10, 5.25, "Boots"}} };
	lovela::named_tuple<l_tuple_names_2, std::tuple<double, double, double>> obj2{ {{7.75, 1.25, 0.1}} };

	auto cat = std::tuple_cat(obj1.as_tuple(), obj2.as_tuple());
	lovela::named_tuple<l_tuple_names_12, decltype(cat)> obj12{ {cat} };

	double v1{};
	EXPECT_NO_THROW(obj12.get_item(2, v1));
	EXPECT_EQ(v1, 5.25);
	EXPECT_NO_THROW(obj12.get_item(5, v1));
	EXPECT_EQ(v1, 1.25);
	EXPECT_NO_THROW(obj12.get_item(u8"Price", v1));
	EXPECT_EQ(v1, 5.25);
	EXPECT_NO_THROW(obj12.get_item(u8"Tax", v1));
	EXPECT_EQ(v1, 1.25);
}

TEST(IndexRebase, TestRebaseAndRange) {
	EXPECT_EQ(lovela::detail::rebase(1, 10), 0);
	EXPECT_EQ(lovela::detail::rebase(10, 10), 9);
	EXPECT_THROW(static_cast<void>(lovela::detail::rebase(0, 10)), std::out_of_range);
	EXPECT_THROW(static_cast<void>(lovela::detail::rebase(11, 10)), std::out_of_range);
}

TEST(FixedTuple, InitAndRange) {
	lovela::fixed_tuple<std::tuple<int, double, std::string>> obj1;
	EXPECT_NO_THROW(obj1.set_size(3));
	EXPECT_THROW(obj1.set_size(20), std::out_of_range);
	EXPECT_EQ(obj1.get_size(), 3);
	EXPECT_THROW(obj1.add_item(123), std::out_of_range);
}

TEST(FixedTuple, SetGet) {
	lovela::fixed_tuple<std::tuple<int, double, std::string>> obj1;
	EXPECT_NO_THROW(obj1.set_item<1>(123));
	EXPECT_NO_THROW(obj1.set_item<2>(456.789));
	EXPECT_NO_THROW(obj1.set_item<3>(std::string("abc")));

	int v1{};
	EXPECT_NO_THROW(obj1.get_item<1>(v1));
	EXPECT_EQ(v1, 123);
	EXPECT_NO_THROW(obj1.get_item(1, v1));
	EXPECT_EQ(v1, 123);
	EXPECT_NO_THROW(obj1.get_item(u8"1", v1));
	EXPECT_EQ(v1, 123);

	double v2{};
	EXPECT_NO_THROW(obj1.get_item<2>(v2));
	EXPECT_EQ(v2, 456.789);
	EXPECT_NO_THROW(obj1.get_item(2, v2));
	EXPECT_EQ(v2, 456.789);
	EXPECT_NO_THROW(obj1.get_item(u8"2", v2));
	EXPECT_EQ(v2, 456.789);

	std::string v3{};
	EXPECT_NO_THROW(obj1.get_item<3>(v3));
	EXPECT_EQ(v3, "abc");
	EXPECT_NO_THROW(obj1.get_item(3, v3));
	EXPECT_EQ(v3, "abc");
	EXPECT_NO_THROW(obj1.get_item(u8"3", v3));
	EXPECT_EQ(v3, "abc");
}

TEST(FixedTuple, Range) {
	lovela::fixed_tuple<std::tuple<int, double, std::string>> obj1;
	int v1{};
	EXPECT_THROW(obj1.get_item(3, v1), std::invalid_argument);
	EXPECT_THROW(obj1.get_item(0, v1), std::out_of_range);
	EXPECT_THROW(obj1.get_item(4, v1), std::out_of_range);
	EXPECT_THROW(obj1.get_item(u8"", v1), std::invalid_argument);
	EXPECT_THROW(obj1.get_item(u8"null", v1), std::invalid_argument);
	EXPECT_THROW(obj1.get_item(u8"0", v1), std::out_of_range);
	EXPECT_THROW(obj1.get_item(u8"4", v1), std::out_of_range);
}

TEST(FixedTuple, GetIndex) {
	lovela::fixed_tuple<std::tuple<int, double, std::string>> obj1;
	EXPECT_EQ(obj1.get_index(u8"1"), 1);
	EXPECT_THROW(obj1.get_index(u8""), std::invalid_argument);
	EXPECT_THROW(obj1.get_index(u8"null"), std::invalid_argument);
	EXPECT_THROW(obj1.get_index(u8"0"), std::out_of_range);
	EXPECT_THROW(obj1.get_index(u8"4"), std::out_of_range);
}

TEST(NamedTuple, InitAndRange) {
	lovela::named_tuple<l_tuple_names_1, std::tuple<int, double, std::string>> obj1;
	EXPECT_NO_THROW(obj1.set_size(3));
	EXPECT_THROW(obj1.set_size(20), std::out_of_range);
	EXPECT_EQ(obj1.get_size(), 3);
	EXPECT_THROW(obj1.add_item(123), std::out_of_range);
}

TEST(NamedTuple, SetGet) {
	lovela::named_tuple<l_tuple_names_1, std::tuple<int, double, std::string>> obj1;
	EXPECT_NO_THROW(obj1.set_item<1>(123));
	EXPECT_NO_THROW(obj1.set_item<2>(456.789));
	EXPECT_NO_THROW(obj1.set_item<3>(std::string("abc")));

	int v1{};
	EXPECT_NO_THROW(obj1.get_item<1>(v1));
	EXPECT_EQ(v1, 123);
	EXPECT_NO_THROW(obj1.get_item(1, v1));
	EXPECT_EQ(v1, 123);
	EXPECT_NO_THROW(obj1.get_item(u8"1", v1));
	EXPECT_EQ(v1, 123);

	double v2{};
	EXPECT_NO_THROW(obj1.get_item<2>(v2));
	EXPECT_EQ(v2, 456.789);
	EXPECT_NO_THROW(obj1.get_item(2, v2));
	EXPECT_EQ(v2, 456.789);
	EXPECT_NO_THROW(obj1.get_item(u8"2", v2));
	EXPECT_EQ(v2, 456.789);

	std::string v3{};
	EXPECT_NO_THROW(obj1.get_item<3>(v3));
	EXPECT_EQ(v3, "abc");
	EXPECT_NO_THROW(obj1.get_item(3, v3));
	EXPECT_EQ(v3, "abc");
	EXPECT_NO_THROW(obj1.get_item(u8"3", v3));
	EXPECT_EQ(v3, "abc");
}

TEST(NamedTuple, Range) {
	lovela::named_tuple<l_tuple_names_1, std::tuple<int, double, std::string>> obj1;
	int v1{};
	EXPECT_THROW(obj1.get_item(3, v1), std::invalid_argument);
	EXPECT_THROW(obj1.get_item(0, v1), std::out_of_range);
	EXPECT_THROW(obj1.get_item(4, v1), std::out_of_range);
	EXPECT_THROW(obj1.get_item(u8"", v1), std::invalid_argument);
	EXPECT_THROW(obj1.get_item(u8"null", v1), std::invalid_argument);
	EXPECT_THROW(obj1.get_item(u8"0", v1), std::out_of_range);
	EXPECT_THROW(obj1.get_item(u8"4", v1), std::out_of_range);
}

TEST(NamedTuple, GetIndex) {
	lovela::named_tuple<l_tuple_names_1, std::tuple<int, double, std::string>> obj1;
	EXPECT_EQ(obj1.get_index(u8"1"), 1);
	EXPECT_THROW(obj1.get_index(u8""), std::invalid_argument);
	EXPECT_THROW(obj1.get_index(u8"null"), std::invalid_argument);
	EXPECT_THROW(obj1.get_index(u8"0"), std::out_of_range);
	EXPECT_THROW(obj1.get_index(u8"4"), std::out_of_range);
}

TEST(NamedTuple, SetGetRuntimeName) {
	lovela::named_tuple<l_tuple_names_1, std::tuple<int, double, std::string>> obj1;
	EXPECT_NO_THROW(obj1.set_item(u8"Pcs", 123));
	EXPECT_NO_THROW(obj1.set_item(u8"Price", 123.456));
	EXPECT_NO_THROW(obj1.set_item(u8"Name", std::string("abc")));
	EXPECT_THROW(obj1.set_item(u8"Type", 123), std::invalid_argument);

	int v1{}; double v2{}; std::string v3;
	EXPECT_NO_THROW(obj1.get_item(u8"Pcs", v1));
	EXPECT_NO_THROW(obj1.get_item(u8"Price", v2));
	EXPECT_NO_THROW(obj1.get_item(u8"Name", v3));
	EXPECT_THROW(obj1.get_item(u8"Type", v1), std::invalid_argument);
	EXPECT_EQ(v1, 123);
	EXPECT_EQ(v2, 123.456);
	EXPECT_EQ(v3, std::string("abc"));
}

TEST(NamedTuple, GetRuntimeRandomAccess) {
	lovela::named_tuple<l_tuple_names_1, std::tuple<int, double, std::string>> obj1;
	EXPECT_NO_THROW(obj1.set_item<1>(123));
	EXPECT_NO_THROW(obj1.set_item<2>(123.456));
	EXPECT_NO_THROW(obj1.set_item<3>(std::string("abc")));

	size_t rnd = rand() % 3;
	auto val = l_tuple_names_1::values[rnd];
	std::u8string name(val.data(), val.size());

	int v1{}; double v2{}; std::string v3;
	switch (rnd)
	{
	case 0:
		EXPECT_NO_THROW(obj1.get_item(name, v1));
		EXPECT_EQ(v1, 123);
		break;

	case 1:
		EXPECT_NO_THROW(obj1.get_item(name, v2));
		EXPECT_EQ(v2, 123.456);
		break;

	case 2:
		EXPECT_NO_THROW(obj1.get_item(name, v3));
		EXPECT_EQ(v3, std::string("abc"));
		break;

	default:
		EXPECT_EQ(0, 1);
		break;
	}
}

struct l_tuple_names_totalRest
{
	static constexpr auto values = std::array<std::u8string_view, 2>{ u8"Total", u8"Rest" };
};

struct l_tuple_names_restTotal
{
	static constexpr auto values = std::array<std::u8string_view, 2>{ u8"Rest", u8"Total" };
};

TEST(NamedTuple, CoexistingTuples) {
	lovela::named_tuple<l_tuple_names_totalRest, std::tuple<int, int>> totalRest;
	lovela::named_tuple<l_tuple_names_restTotal, std::tuple<int, int>> restTotal;

	EXPECT_NO_THROW(totalRest.set_item(u8"Total", 111));
	EXPECT_NO_THROW(totalRest.set_item(u8"Rest", 100));
	EXPECT_NO_THROW(restTotal.set_item(u8"Total", 222));
	EXPECT_NO_THROW(restTotal.set_item(u8"Rest", 200));

	int v1{};
	EXPECT_NO_THROW(totalRest.get_item(u8"Total", v1));
	EXPECT_EQ(v1, 111);
	EXPECT_NO_THROW(totalRest.get_item(u8"Rest", v1));
	EXPECT_EQ(v1, 100);
	EXPECT_NO_THROW(restTotal.get_item(u8"Total", v1));
	EXPECT_EQ(v1, 222);
	EXPECT_NO_THROW(restTotal.get_item(u8"Rest", v1));
	EXPECT_EQ(v1, 200);

	EXPECT_NO_THROW(totalRest.get_item(1, v1));
	EXPECT_EQ(v1, 111);
	EXPECT_NO_THROW(totalRest.get_item(2, v1));
	EXPECT_EQ(v1, 100);
	EXPECT_NO_THROW(restTotal.get_item(2, v1));
	EXPECT_EQ(v1, 222);
	EXPECT_NO_THROW(restTotal.get_item(1, v1));
	EXPECT_EQ(v1, 200);
}

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

	std::string v1;
	EXPECT_NO_THROW(obj1.set_item<10>("aaa"));
	EXPECT_NO_THROW(obj1.get_item<10>(v1));
	EXPECT_EQ(v1, "aaa");

	int v2{};
	EXPECT_NO_THROW(obj2.set_item<10>(123));
	EXPECT_NO_THROW(obj2.get_item<10>(v2));
	EXPECT_EQ(v2, 123);
}

TEST(DynamicArray, SetGetAddRange) {
	lovela::dynamic_array<std::string> obj1;

	EXPECT_NO_THROW(obj1.set_size(20));

	std::string v1;
	EXPECT_NO_THROW(obj1.set_item<1>("aaa"));
	EXPECT_NO_THROW(obj1.get_item<1>(v1));
	EXPECT_EQ(v1, "aaa");
	EXPECT_NO_THROW(obj1.get_item(1, v1));
	EXPECT_EQ(v1, "aaa");
	EXPECT_NO_THROW(obj1.get_item(u8"1", v1));
	EXPECT_EQ(v1, "aaa");

	EXPECT_NO_THROW(obj1.set_item(1, "bbb"));
	EXPECT_NO_THROW(obj1.get_item<1>(v1));
	EXPECT_EQ(v1, "bbb");
	EXPECT_NO_THROW(obj1.get_item(1, v1));
	EXPECT_EQ(v1, "bbb");
	EXPECT_NO_THROW(obj1.get_item(u8"1", v1));
	EXPECT_EQ(v1, "bbb");

	EXPECT_NO_THROW(obj1.set_item(u8"1", "ccc"));
	EXPECT_NO_THROW(obj1.get_item<1>(v1));
	EXPECT_EQ(v1, "ccc");
	EXPECT_NO_THROW(obj1.get_item(1, v1));
	EXPECT_EQ(v1, "ccc");
	EXPECT_NO_THROW(obj1.get_item(u8"1", v1));
	EXPECT_EQ(v1, "ccc");

	EXPECT_NO_THROW(obj1.set_item(20, "bcd"));
	EXPECT_NO_THROW(obj1.get_item(20, v1));
	EXPECT_EQ(v1, "bcd");

	EXPECT_THROW(obj1.get_item(0, v1), std::out_of_range);
	EXPECT_THROW(obj1.set_item(21, "cde"), std::out_of_range);

	EXPECT_NO_THROW(obj1.add_item("def"));
	EXPECT_NO_THROW(obj1.get_item(21, v1));
	EXPECT_EQ(v1, "def");
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

	std::string v1;
	EXPECT_NO_THROW(obj1.set_item<10>("aaa"));
	EXPECT_NO_THROW(obj1.get_item<10>(v1));
	EXPECT_EQ(v1, "aaa");

	int v2{};
	EXPECT_NO_THROW(obj2.set_item<10>(123));
	EXPECT_NO_THROW(obj2.get_item<10>(v2));
	EXPECT_EQ(v2, 123);
}

TEST(FixedArray, SetGetAddRange) {
	lovela::fixed_array<std::string, 20> obj1;

	EXPECT_NO_THROW(obj1.set_size(20));

	std::string v1;
	EXPECT_NO_THROW(obj1.set_item<1>("aaa"));
	EXPECT_NO_THROW(obj1.get_item<1>(v1));
	EXPECT_EQ(v1, "aaa");
	EXPECT_NO_THROW(obj1.get_item(1, v1));
	EXPECT_EQ(v1, "aaa");
	EXPECT_NO_THROW(obj1.get_item(u8"1", v1));
	EXPECT_EQ(v1, "aaa");

	EXPECT_NO_THROW(obj1.set_item(1, "bbb"));
	EXPECT_NO_THROW(obj1.get_item<1>(v1));
	EXPECT_EQ(v1, "bbb");
	EXPECT_NO_THROW(obj1.get_item(1, v1));
	EXPECT_EQ(v1, "bbb");
	EXPECT_NO_THROW(obj1.get_item(u8"1", v1));
	EXPECT_EQ(v1, "bbb");

	EXPECT_NO_THROW(obj1.set_item(u8"1", "ccc"));
	EXPECT_NO_THROW(obj1.get_item<1>(v1));
	EXPECT_EQ(v1, "ccc");
	EXPECT_NO_THROW(obj1.get_item(1, v1));
	EXPECT_EQ(v1, "ccc");
	EXPECT_NO_THROW(obj1.get_item(u8"1", v1));
	EXPECT_EQ(v1, "ccc");

	EXPECT_NO_THROW(obj1.set_item(20, "bcd"));
	EXPECT_NO_THROW(obj1.get_item(20, v1));
	EXPECT_EQ(v1, "bcd");

	EXPECT_THROW(obj1.get_item(0, v1), std::out_of_range);
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

TEST(Streams, SimpleOut) {
	std::wostringstream v1;
	auto* buf = std::wcout.rdbuf(v1.rdbuf());
	lovela::streams streams;
	streams.select(2).write("abc");
	EXPECT_STREQ(v1.str().c_str(), L"abc");
	std::wcout.rdbuf(buf);
}

TEST(Streams, Utf8Out) {
	std::wostringstream v1;
	auto* buf = std::wcout.rdbuf(v1.rdbuf());
	lovela::streams streams;
	streams.select(2).write("100\xE2\x82\xAC");
	EXPECT_STREQ(v1.str().c_str(), L"100€");
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
	auto& v1 = in; v1;
	return v1;
}

auto f_ReturnInputIncremented(lovela::context& context, const auto& in)
{
	context;
	auto& v1 = in; v1;
	auto v2 = v1 + 1;
	return v2;
}

#pragma warning(push)
#pragma warning(disable: 4702)
auto f_RaisesError(lovela::context& context, const auto& in)
{
	context;
	auto& v1 = in; v1;
	throw lovela::error("error");
	return v1;
}
#pragma warning(pop)

auto f_ErrorHandlerReset(lovela::context& context, const auto& in)
{
	context;
	auto& v1 = in; v1;
	return v1;
}

auto fb_WithTailErrorHandlerOnError(lovela::context& context, const auto& in)
{
	context;
	auto& v1 = in; v1;
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
	context;
	auto& v1 = in; v1;
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
	context;
	auto& v1 = in; v1;
	auto v2 = v1 + 10;
	auto v3 = f_RaisesError(context, v2);
	return v3;
}

auto fb2_WithMidErrorHandlerOnError(lovela::context& context, const auto& in)
{
	context;
	auto& v1 = in; v1;
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
	context;
	auto& v1 = in; v1;
	auto v2 = v1 + 10;
	return v2;
}

auto fb2_WithMidErrorHandlerOnSuccess(lovela::context& context, const auto& in)
{
	context;
	auto& v1 = in; v1;
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


lovela::None lovela::main(lovela::context& context, lovela::None in)
{
	context;
	auto& v1 = in; v1;
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
