#include "pch.h"
#include "test.h"

TEST(IndexRebase, StaticAndDynamic) {
	EXPECT_EQ(lovela::detail::rebase(1, 10), 0);
	EXPECT_EQ(lovela::detail::rebase(10, 10), 9);
	EXPECT_THROW(lovela::detail::rebase(0, 10), std::out_of_range);
	EXPECT_THROW(lovela::detail::rebase(11, 10), std::out_of_range);
	EXPECT_EQ(lovela::detail::rebase_v<1>, 0);
	EXPECT_EQ(lovela::detail::rebase_v<10>, 9);
}

TEST(IndexedTuple, InitAndRange) {
	lovela::indexed_tuple<int, double, std::string> obj;
	EXPECT_NO_THROW(obj.set_size(3));
	EXPECT_THROW(obj.set_size(20), std::out_of_range);
	EXPECT_EQ(obj.get_size(), 3);
	EXPECT_THROW(obj.add_item(123), std::out_of_range);
}

TEST(IndexedTuple, SetGetStaticIndex) {
	lovela::indexed_tuple<int, double, std::string> obj;
	EXPECT_NO_THROW(obj.set_item<1>(123));
	EXPECT_NO_THROW(obj.set_item<2>(123.456));
	EXPECT_NO_THROW(obj.set_item<3>(std::string("abc")));

	int i{}; double d{}; std::string s;
	EXPECT_NO_THROW(obj.get_item<1>(i));
	EXPECT_NO_THROW(obj.get_item<2>(d));
	EXPECT_NO_THROW(obj.get_item<3>(s));
	EXPECT_THROW(obj.get_item(4, i), std::out_of_range);
	EXPECT_EQ(i, 123);
	EXPECT_EQ(d, 123.456);
	EXPECT_EQ(s, std::string("abc"));
}

TEST(IndexedTuple, SetGetRuntimeIndex) {
	lovela::indexed_tuple<int, double, std::string> obj;
	EXPECT_NO_THROW(obj.set_item(1, 123));
	EXPECT_NO_THROW(obj.set_item(2, 123.456));
	EXPECT_NO_THROW(obj.set_item(3, std::string("abc")));
	EXPECT_THROW(obj.set_item(4, 123), std::out_of_range);

	int i{}; double d{}; std::string s;
	EXPECT_NO_THROW(obj.get_item(1, i));
	EXPECT_NO_THROW(obj.get_item(2, d));
	EXPECT_NO_THROW(obj.get_item(3, s));
	EXPECT_THROW(obj.get_item(4, i), std::out_of_range);
	EXPECT_EQ(i, 123);
	EXPECT_EQ(d, 123.456);
	EXPECT_EQ(s, std::string("abc"));
}

template <>
struct lovela::named_tuple_names<1>
{
	static constexpr std::array<std::u8string_view, 3> names{ u8"Pcs", u8"Price", u8"Name" };
};

TEST(NamedTuple, InitAndRange) {
	lovela::named_tuple<1, int, double, std::string> obj;
	EXPECT_NO_THROW(obj.set_size(3));
	EXPECT_THROW(obj.set_size(20), std::out_of_range);
	EXPECT_EQ(obj.get_size(), 3);
	EXPECT_THROW(obj.add_item(123), std::out_of_range);
}

TEST(NamedTuple, SetGetStaticIndex) {
	lovela::named_tuple<1, int, double, std::string> obj;
	EXPECT_NO_THROW(obj.set_item<1>(123));
	EXPECT_NO_THROW(obj.set_item<2>(123.456));
	EXPECT_NO_THROW(obj.set_item<3>(std::string("abc")));

	int i{}; double d{}; std::string s;
	EXPECT_NO_THROW(obj.get_item<1>(i));
	EXPECT_NO_THROW(obj.get_item<2>(d));
	EXPECT_NO_THROW(obj.get_item<3>(s));
	EXPECT_THROW(obj.get_item(4, i), std::out_of_range);
	EXPECT_EQ(i, 123);
	EXPECT_EQ(d, 123.456);
	EXPECT_EQ(s, std::string("abc"));
}

TEST(NamedTuple, SetGetRuntimeIndex) {
	lovela::named_tuple<1, int, double, std::string> obj;
	EXPECT_NO_THROW(obj.set_item(1, 123));
	EXPECT_NO_THROW(obj.set_item(2, 123.456));
	EXPECT_NO_THROW(obj.set_item(3, std::string("abc")));
	EXPECT_THROW(obj.set_item(4, 123), std::out_of_range);

	int i{}; double d{}; std::string s;
	EXPECT_NO_THROW(obj.get_item(1, i));
	EXPECT_NO_THROW(obj.get_item(2, d));
	EXPECT_NO_THROW(obj.get_item(3, s));
	EXPECT_THROW(obj.get_item(4, i), std::out_of_range);
	EXPECT_EQ(i, 123);
	EXPECT_EQ(d, 123.456);
	EXPECT_EQ(s, std::string("abc"));
}

TEST(NamedTuple, SetGetRuntimeName) {
	lovela::named_tuple<1, int, double, std::string> obj;
	EXPECT_NO_THROW(obj.set_item(u8"Pcs", 123));
	EXPECT_NO_THROW(obj.set_item(u8"Price", 123.456));
	EXPECT_NO_THROW(obj.set_item(u8"Name", std::string("abc")));
	EXPECT_THROW(obj.set_item(u8"Type", 123), std::out_of_range);

	int i{}; double d{}; std::string s;
	EXPECT_NO_THROW(obj.get_item(u8"Pcs", i));
	EXPECT_NO_THROW(obj.get_item(u8"Price", d));
	EXPECT_NO_THROW(obj.get_item(u8"Name", s));
	EXPECT_THROW(obj.get_item(u8"Type", i), std::out_of_range);
	EXPECT_EQ(i, 123);
	EXPECT_EQ(d, 123.456);
	EXPECT_EQ(s, std::string("abc"));
}

TEST(NamedTuple, GetRuntimeRandomAccess) {
	lovela::named_tuple<1, int, double, std::string> obj;
	EXPECT_NO_THROW(obj.set_item<1>(123));
	EXPECT_NO_THROW(obj.set_item<2>(123.456));
	EXPECT_NO_THROW(obj.set_item<3>(std::string("abc")));

	size_t rnd = rand() % 3;
	auto val = lovela::named_tuple_names<1>::names[rnd];
	std::u8string name(val.data(), val.size());

	int i{}; double d{}; std::string s;
	switch (rnd)
	{
	case 0:
		EXPECT_NO_THROW(obj.get_item(name, i));
		EXPECT_EQ(i, 123);
		break;

	case 1:
		EXPECT_NO_THROW(obj.get_item(name, d));
		EXPECT_EQ(d, 123.456);
		break;

	case 2:
		EXPECT_NO_THROW(obj.get_item(name, s));
		EXPECT_EQ(s, std::string("abc"));
		break;

	default:
		EXPECT_EQ(0, 1);
		break;
	}
}

static constexpr size_t NamedTupleTotalRest = 2;

template <>
struct lovela::named_tuple_names<NamedTupleTotalRest>
{
	static constexpr std::array<std::u8string_view, 2> names{ u8"Total", u8"Rest" };
};

static constexpr size_t NamedTupleRestTotal = 3;

template <>
struct lovela::named_tuple_names<NamedTupleRestTotal>
{
	static constexpr std::array<std::u8string_view, 2> names{ u8"Rest", u8"Total" };
};

TEST(NamedTuple, CoexistingTuples) {
	lovela::named_tuple<NamedTupleTotalRest, int, int> totalRest;
	lovela::named_tuple<NamedTupleRestTotal, int, int> restTotal;

	EXPECT_NO_THROW(totalRest.set_item(u8"Total", 111));
	EXPECT_NO_THROW(totalRest.set_item(u8"Rest", 100));
	EXPECT_NO_THROW(restTotal.set_item(u8"Total", 222));
	EXPECT_NO_THROW(restTotal.set_item(u8"Rest", 200));

	int i{};
	EXPECT_NO_THROW(totalRest.get_item(u8"Total", i));
	EXPECT_EQ(i, 111);
	EXPECT_NO_THROW(totalRest.get_item(u8"Rest", i));
	EXPECT_EQ(i, 100);
	EXPECT_NO_THROW(restTotal.get_item(u8"Total", i));
	EXPECT_EQ(i, 222);
	EXPECT_NO_THROW(restTotal.get_item(u8"Rest", i));
	EXPECT_EQ(i, 200);

	EXPECT_NO_THROW(totalRest.get_item(1, i));
	EXPECT_EQ(i, 111);
	EXPECT_NO_THROW(totalRest.get_item(2, i));
	EXPECT_EQ(i, 100);
	EXPECT_NO_THROW(restTotal.get_item(2, i));
	EXPECT_EQ(i, 222);
	EXPECT_NO_THROW(restTotal.get_item(1, i));
	EXPECT_EQ(i, 200);
}

TEST(DynamicArray, SetGetAddRange) {
	lovela::dynamic_array<std::string> arr;
	EXPECT_NO_THROW(arr.set_size(10));
	EXPECT_NO_THROW(arr.set_size(20));
	EXPECT_EQ(arr.get_size(), 20);
	EXPECT_NO_THROW(arr.set_item(1, "abc"));
	std::string s;
	EXPECT_NO_THROW(arr.get_item(1, s));
	EXPECT_EQ(s, "abc");
	EXPECT_NO_THROW(arr.set_item(20, "bcd"));
	EXPECT_NO_THROW(arr.get_item(20, s));
	EXPECT_EQ(s, "bcd");
	EXPECT_THROW(arr.get_item(0, s), std::out_of_range);
	EXPECT_THROW(arr.set_item(21, "cde"), std::out_of_range);
	EXPECT_NO_THROW(arr.add_item("def"));
	EXPECT_NO_THROW(arr.get_item(21, s));
	EXPECT_EQ(s, "def");
	EXPECT_EQ(arr.get_size(), 21);
}

TEST(FixedSizeArray, SetGetAddRange) {
	lovela::fixed_array<int> arr(10);
	EXPECT_NO_THROW(arr.set_size(10));
	EXPECT_THROW(arr.set_size(20), std::out_of_range);
	EXPECT_EQ(arr.get_size(), 10);
	EXPECT_NO_THROW(arr.set_item(1, 123));
	int i{};
	EXPECT_NO_THROW(arr.get_item(1, i));
	EXPECT_EQ(i, 123);
	EXPECT_NO_THROW(arr.set_item(10, 234));
	EXPECT_NO_THROW(arr.get_item(10, i));
	EXPECT_EQ(i, 234);
	EXPECT_THROW(arr.get_item(0, i), std::out_of_range);
	EXPECT_THROW(arr.set_item(11, 345), std::out_of_range);
	EXPECT_THROW(arr.add_item(123), std::out_of_range);
	EXPECT_EQ(arr.get_size(), 10);
}

TEST(Streams, SimpleOut) {
	std::wostringstream s;
	auto* buf = std::wcout.rdbuf(s.rdbuf());
	lovela::streams streams;
	streams.select(2).write("abc");
	EXPECT_STREQ(s.str().c_str(), L"abc");
	std::wcout.rdbuf(buf);
}

TEST(Streams, Utf8Out) {
	std::wostringstream s;
	auto* buf = std::wcout.rdbuf(s.rdbuf());
	lovela::streams streams;
	streams.select(2).write("100\xE2\x82\xAC");
	EXPECT_STREQ(s.str().c_str(), L"100€");
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
