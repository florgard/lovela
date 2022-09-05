#include "pch.h"
#include "../targets/cpp/source/lovela.h"

suite Stream = [] {
	"SimpleOut"_test = [] {
		std::wostringstream r1;
		auto* buf = std::wcout.rdbuf(r1.rdbuf());
		lovela::streams streams;
		streams.select(2).write("abc");
		expect(r1.str() == L"abc");
		std::wcout.rdbuf(buf);
	};

	"Utf8Out"_test = [] {
		std::wostringstream r1;
		auto* buf = std::wcout.rdbuf(r1.rdbuf());
		lovela::streams streams;
		streams.select(2).write("100\xE2\x82\xAC");
		expect(!std::wcscmp(r1.str().c_str(), L"100€"));
		std::wcout.rdbuf(buf);
	};
};

suite Error = [] {
	"Error"_test = [] {
		lovela::error e1("msg");
		lovela::error e2("msg", 2);
		lovela::error e3("msg", "type", 3);
		std::runtime_error rte("rte");
		lovela::error e4 = lovela::error::make_error(rte, 4);

		expect(e1.code == 0);
		expect(e1.message == "msg");
		expect(e2.code == 2);
		expect(e3.type == "type");
		expect(e3.code == 3);
		expect(e4.message == "rte");
		expect(e4.type == typeid(std::runtime_error).name());
		expect(e4.code == 4);
		expect(std::string{ e4.inner.what() } == "rte");
		expect(e4.select<1>() == "rte");
		expect(e4.select<2>() == typeid(std::runtime_error).name());
		expect(e4.select<3>() == 4);
		expect(std::string{ e4.select<4>().what() } == "rte");
	};
};

suite Context = [] {
	"Context"_test = [] {
		lovela::context context{ .error{"msg"}, .parameters{"param"} };
		expect(context.select<2>().select<1>() == "msg");
		expect(context.select<4>().front() == "param");
	};
};
