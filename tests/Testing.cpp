#include "pch.h"
#include "Testing.h"
#include "../lovela/LexerFactory.h"
#include "../lovela/ParserFactory.h"
#include "../lovela/CodeGeneratorFactory.h"
#include "../lovela/LexerBase.h"
#include "../lovela/Algorithm.h"

void Testing::RunParserTests()
{


	{
		auto f = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .parameters{
				make<VariableDeclaration>::shared({.name = L"name_only"}),
				make<VariableDeclaration>::shared({.name = L"name", .type{.name = L"type"}}),
				make<VariableDeclaration>::shared({.type{.name = L"type_only"}})
			} };
		TestParser("function with parameters", L"func(name_only, name [type], [type_only])", f);
	}

	{
		auto f = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .outType{.name = L"functionType"}, .inType{.name = L"inType"}, .parameters{
				make<VariableDeclaration>::shared({.name = L"name_only"}),
				make<VariableDeclaration>::shared({.name = L"name", .type{.name = L"type"}}),
				make<VariableDeclaration>::shared({.type{.name = L"type_only"}})
			} };
		TestParser("complete function declaration", L"[inType] func (name_only, name [type], [type_only]) [functionType]", f);
	}

	TestParser("imported function", L"-> func",
		Node{.type = Node::Type::FunctionDeclaration, .value = L"func", .api = Api::Import}
		);
	TestParser("exported function", L"<- [] func",
		Node{.type = Node::Type::FunctionDeclaration, .value = L"func", .api = Api::Export}
		);
	TestParser("imported C function", L"-> 'C' func",
		Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .api = Api::Import | Api::C }
	);
	TestParser("imported dynamically linked C function", L"-> 'C Dynamic' func",
		Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .api = Api::Import | Api::C | Api::Dynamic }
	);
	TestParser("imported standard C++ function", L"-> 'Standard C++' func",
		Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .api = Api::Import | Api::Cpp | Api::Standard }
	);

	{
		auto f = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .nameSpace{ L"namespace" } };
		TestParser("function with 1 namespace", L"namespace|func", f);
	}
	{
		auto f = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .nameSpace{ L"namespace1", L"namespaceN" } };
		TestParser("function with 2 namespaces", L"namespace1|namespaceN|func", f);
	}
	{
		auto f = Node{ .type = Node::Type::FunctionDeclaration, .value = L"<", .parameters{
				make<VariableDeclaration>::shared({.name = L"operand"}),
			} };
		TestParser("binary operator", L"<(operand)", f);
	}
	{
		auto f = Node{ .type = Node::Type::FunctionDeclaration, .value = L"<", .nameSpace{ L"namespace" }, .parameters{
				make<VariableDeclaration>::shared({.name = L"operand"}),
			} };
		TestParser("binary operator with namespace", L"namespace|< (operand)", f);
	}
	{
		auto f2 = Node{ .type = Node::Type::FunctionDeclaration, .value = L"<", .nameSpace{ L"namespace1" } };
		auto f1 = Node{ .type = Node::Type::FunctionDeclaration, .value = L"namespace2", .parameters{
				make<VariableDeclaration>::shared({.name = L"operand"}),
			}, .right = make<Node>::unique(f2) };
		TestParser("invalid binary operator as namespace", L"namespace1|<|namespace2 (operand)", f1, { IParser::Error{.code = IParser::Error::Code::ParseError } });
	}

	{
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = L"body", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(fc) };
		TestParser("function with trivial body", L"func: body.", fd);
	}

	{
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = L"inner", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = L"outer", .left = make<Node>::unique(fc2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(fc1) };
		TestParser("function with 2 chained calls", L"func: inner outer.", fd);
	}

	{
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = L"body", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(fc) };
		TestParser("function with group", L"func: (body).", fd);
		TestParser("function with group 2", L"func: (body.).", fd);
	}

	{
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = L"expr2", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fc1  = Node{ .type = Node::Type::FunctionCall, .value = L"expr1", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}), .right = make<Node>::unique(fc2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(fc1) };
		TestParser("function with compound expression", L"func: (expr1. expr2).", fd);
	}

	{
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = L"expr2", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = L"expr1", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto t = Node{ .type = Node::Type::Tuple, .left = make<Node>::unique(fc1), .right = make<Node>::unique(fc2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(t) };
		TestParser("function with tuple", L"func: (expr1, expr2).", fd);
	}

	{
		auto fc3 = Node{ .type = Node::Type::FunctionCall, .value = L"expr3", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = L"expr2", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto t2 = Node{ .type = Node::Type::Tuple, .left = make<Node>::unique(fc2), .right = make<Node>::unique(fc3) };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = L"expr1", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto t1 = Node{ .type = Node::Type::Tuple, .left = make<Node>::unique(fc1), .right = make<Node>::unique(t2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(t1) };
		TestParser("function with triple", L"func: (expr1, expr2, expr3).", fd);
	}

	{
		auto fc3 = Node{ .type = Node::Type::FunctionCall, .value = L"expr2b", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = L"expr2a", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}), .right = make<Node>::unique(fc3) };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = L"expr1", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto t = Node{ .type = Node::Type::Tuple, .left = make<Node>::unique(fc1), .right = make<Node>::unique(fc2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(t) };
		TestParser("function with tuple and compound expression", L"func: (expr1, expr2a. expr2b).", fd);
	}

	{
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = L"doWork", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .parameters{
				make<VariableDeclaration>::shared({.name = L"name_only"}),
				make<VariableDeclaration>::shared({.name = L"name", .type{.name = L"type"}}),
				make<VariableDeclaration>::shared({.type{.name = L"type_only"}})
			}, .left = make<Node>::unique(fc) };
		TestParser("function with parameters and body", L"func(name_only, name [type], [type_only]): doWork.", fd);
	}

	{
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = L"doWork", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .inType = TypeSpec::NoneType(), .parameters{
				make<VariableDeclaration>::shared({.name = L"name_only"}),
				make<VariableDeclaration>::shared({.name = L"name", .type{.name = L"type"}}),
				make<VariableDeclaration>::shared({.type{.name = L"type_only"}})
			}, .left = make<Node>::unique(fc) };
		TestParser("function without object but with parameters and body", L"[()] func(name_only, name [type], [type_only]): doWork.", fd);
	}

	{
		auto l = Node{ .type = Node::Type::Literal, .value = L"1", .outType{.name = L"#32"} };
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = L"call", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto bo = Node{ .type = Node::Type::BinaryOperation, .value = L"+", .left = make<Node>::unique(fc), .right = make<Node>::unique(l) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(bo) };
		TestParser("binary operation with function call", L"func: call + 1.", fd);
	}
}

void Testing::RunCodeGeneratorTests()
{
	TestCodeGenerator("l_i1", L"#1 f", L"auto f_f(lovela::context& context, l_i1 in);");
	TestCodeGenerator("l_i8", L"#8 f", L"auto f_f(lovela::context& context, l_i8 in);");
	TestCodeGenerator("l_i16", L"#16 f", L"auto f_f(lovela::context& context, l_i16 in);");
	TestCodeGenerator("l_i32", L"#32 f", L"auto f_f(lovela::context& context, l_i32 in);");
	TestCodeGenerator("l_i64", L"#64 f", L"auto f_f(lovela::context& context, l_i64 in);");
	TestCodeGenerator("l_u1", L"#+1 f", L"auto f_f(lovela::context& context, l_u1 in);");
	TestCodeGenerator("l_u8", L"#+8 f", L"auto f_f(lovela::context& context, l_u8 in);");
	TestCodeGenerator("l_u16", L"#+16 f", L"auto f_f(lovela::context& context, l_u16 in);");
	TestCodeGenerator("l_u32", L"#+32 f", L"auto f_f(lovela::context& context, l_u32 in);");
	TestCodeGenerator("l_u64", L"#+64 f", L"auto f_f(lovela::context& context, l_u64 in);");
	TestCodeGenerator("l_f16 error", L"#.16 f", L"auto f_f(lovela::context& context, t_#.16 in);", 1);
	TestCodeGenerator("l_f32", L"#.32 f", L"auto f_f(lovela::context& context, l_f32 in);");
	TestCodeGenerator("l_f64", L"#.64 f", L"auto f_f(lovela::context& context, l_f64 in);");

	TestCodeGenerator("trivial function", L"func",
		L"template <typename In> auto f_func(lovela::context& context, In in);");

	TestCodeGenerator("function with return type", L"func [type]",
		L"template <typename In> t_type f_func(lovela::context& context, In in);");

	TestCodeGenerator("function with object type", L"[type] func",
		L"auto f_func(lovela::context& context, t_type in);");

	TestCodeGenerator("function with untyped parameter", L"func (arg)",
		L"template <typename In, typename Param1> auto f_func(lovela::context& context, In in, Param1 p_arg);");

	TestCodeGenerator("function with typed parameter", L"func (arg [type])",
		L"template <typename In> auto f_func(lovela::context& context, In in, t_type p_arg);");

	TestCodeGenerator("trivial function", L"func: + 1.", LR"code(
template <typename In>
auto f_func(lovela::context& context, In in)
{ context; auto& v1 = in; static_cast<void>(v1); const auto v2 = v1 + 1; static_cast<void>(v2); return v2; }
)code");

	TestCodeGenerator("function call", L"[#8] func [#8]: f(1, 'a', g).", LR"code(
l_i8 f_func(lovela::context& context, l_i8 in)
{ context; auto& v1 = in; static_cast<void>(v1); const auto v2 = f_f(context, v1, 1, "a", f_g(context, v1)); static_cast<void>(v2); return v2; }
)code");

	TestCodeGenerator("exported function none -> none", L"<- [()] ex [()]:.", LR"code(
lovela::None f_ex(lovela::context& context, lovela::None in)
{ context; auto& v1 = in; static_cast<void>(v1); return {}; }
void ex()
{ lovela::context context; lovela::None in; f_ex(context, in); }
)code");

	TestCodeGenerator("exported function any -> any", L"<- ex: + 1.", LR"code(
template <typename In>
auto f_ex(lovela::context& context, In in)
{ context; auto& v1 = in; static_cast<void>(v1); const auto v2 = v1 + 1; static_cast<void>(v2); return v2; }
void* ex(void* in)
{ lovela::context context; return f_ex(context, in); }
)code");

	TestCodeGeneratorExport("exported function C", L"<- 'C' #32 ex #32", L"LOVELA_API_C l_i32 ex(l_i32 in)");
	TestCodeGeneratorExport("exported function C++", L"<- 'C++' #32 ex #32", L"LOVELA_API_CPP l_i32 ex(l_i32 in)");
	TestCodeGeneratorExport("exported function C Dynamic", L"<- 'C Dynamic' #32 ex #32", L"LOVELA_API_C LOVELA_API_DYNAMIC_EXPORT l_i32 ex(l_i32 in)");
	TestCodeGenerator("imported function", L"-> im", L"template <typename In> auto f_im(lovela::context& context, In in) { context; return im(in); }");
	TestCodeGenerator("imported function C", L"-> 'C' #8 im #8", L"LOVELA_API_C l_i8 im(l_i8 in); l_i8 f_im(lovela::context& context, l_i8 in) { context; return im(in); }");
	TestCodeGenerator("imported function C Dynamic", L"-> 'C Dynamic' #8 im #8", L"LOVELA_API_C LOVELA_API_DYNAMIC_IMPORT l_i8 im(l_i8 in); l_i8 f_im(lovela::context& context, l_i8 in) { context; return im(in); }");
	TestCodeGeneratorImport("imported function Standard C stdio", L"-> 'Standard C' puts", L"stdio.h");
	TestCodeGeneratorImport("imported function Standard C stlib", L"-> 'Standard C' atof", L"stdlib.h");
	TestCodeGeneratorImport("imported function Standard C string", L"-> 'Standard C' strcpy", L"string.h");
	TestCodeGeneratorImport("imported function Standard C math", L"-> 'Standard C' sin", L"math.h");
	TestCodeGeneratorImport("imported function Standard C++ cstdio", L"-> 'Standard C++' std|puts", L"cstdio");
	TestCodeGeneratorImport("imported function Standard C++ cstlib", L"-> 'Standard C++' std|atof", L"cstdlib");
	TestCodeGeneratorImport("imported function Standard C++ cstring", L"-> 'Standard C++' std|strcpy", L"cstring");
	TestCodeGeneratorImport("imported function Standard C++ cmath", L"-> 'Standard C++' std|sin", L"cmath");

	TestCodeGenerator("main and export", L"<- [#32] ex [#32]: + 1. : 1 ex.", LR"code(
l_i32 f_ex(lovela::context& context, l_i32 in)
{
	context;
	auto& v1 = in; static_cast<void>(v1);
	const auto v2 = v1 + 1; static_cast<void>(v2);
	return v2;
}

l_i32 ex(l_i32 in)
{
	lovela::context context;
	return f_ex(context, in);
}

lovela::None lovela::main(lovela::context& context, lovela::None in)
{
	context;
	auto& v1 = in; static_cast<void>(v1);
	const auto v2 = f_ex(context, 1); static_cast<void>(v2);
	return {};
}
)code");

	TestCodeGenerator("main and explicitly typed import", L"-> [#8#] puts [#32]. : 'Hello, Wordl!' puts.", LR"code(
l_i32 f_puts(lovela::context& context, l_cstr in)
{
        context;
        return puts(in);
}

lovela::None lovela::main(lovela::context& context, lovela::None in)
{
        context;
        auto& v1 = in; static_cast<void>(v1);
        const auto v2 = f_puts(context, "Hello, Wordl!"); static_cast<void>(v2);
        return {};
}
)code");

	TestCodeGenerator("main and implicitly typed import", L"-> puts. : 'Hello, Wordl!' puts.", LR"code(
template <typename In>
auto f_puts(lovela::context& context, In in)
{
	context;
	return puts(in);
}

lovela::None lovela::main(lovela::context& context, lovela::None in)
{
	context;
	auto& v1 = in; static_cast<void>(v1);
	const auto v2 = f_puts(context, "Hello, Wordl!"); static_cast<void>(v2);
	return {};
}
)code");

//	std::wstring code = LR"(
//[()] pi: 3.14.
//mul (factor): * factor.
//transform (mul, sub): (* mul. - sub).
//[](): pi transform (2, 0.28) + 1.
//)";

	// Internal error:
	// -> [#32] puts [#8#]. : 'Hello, Wordl!" puts.

	std::wstring code = LR"(
-> 'Standard C' puts.
: 'Hello, Wordl!' puts.
)";
	std::wcout << code << '\n';

	std::wistringstream input(code);
	auto lexer = LexerFactory::Create(input);
	for (auto& error : lexer->GetErrors())
	{
		std::wcerr << error.message << '\n';
	}

	auto parser = ParserFactory::Create(lexer->Lex());
	auto tree = parser->Parse();
	for (auto& error : parser->GetErrors())
	{
		std::wcerr << error.message << '\n';
	}

	PrintTree(*tree);
	std::wcout << '\n';

	std::wstringstream stream;

	auto codeGen = CodeGeneratorFactory::Create(stream, "Cpp");
	Traverse::DepthFirstPostorder(*tree, [&](Node& node) { codeGen->Visit(node); });

	for (auto& error : codeGen->GetErrors())
	{
		std::wcerr << error << '\n';
	}

	auto genCode = stream.str();
	std::wcout << genCode;

	std::wofstream program(R"(..\targets\cpp\program\lovela-program.cpp)");
	codeGen->GenerateProgramFile(program);
	program.close();

	std::wofstream imports(R"(..\targets\cpp\program\lovela-imports.h)");
	codeGen->GenerateImportsFile(imports);
	imports.close();

	std::wofstream exports(R"(..\targets\cpp\program\lovela-exports.h)");
	codeGen->GenerateExportsFile(exports);
	exports.close();
}
