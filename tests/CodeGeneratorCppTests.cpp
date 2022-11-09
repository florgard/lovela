#include "pch.h"
#include "TestingBase.h"
#include "../lovela/LexerFactory.h"
#include "../lovela/ParserFactory.h"
#include "../lovela/CodeGeneratorFactory.h"

class CodeGenTest : public TestingBase
{
public:
	static bool Success(const char* name, std::wstring_view code, std::wstring_view cppCode)
	{
		return Failure(name, code, cppCode, 0);
	}

	static bool ImportSuccess(const char* name, std::wstring_view code, std::wstring_view cppCode)
	{
		return ImportFailure(name, code, cppCode, 0);
	}

	static bool ExportSuccess(const char* name, std::wstring_view code, std::wstring_view cppCode)
	{
		return ExportFailure(name, code, cppCode, 0);
	}

	static bool Failure(const char* name, std::wstring_view code, std::wstring_view cppCode, int expectedErrors);
	static bool ImportFailure(const char* name, std::wstring_view code, std::wstring_view cppCode, int expectedErrors);
	static bool ExportFailure(const char* name, std::wstring_view code, std::wstring_view cppCode, int expectedErrors);
};

bool CodeGenTest::Failure(const char* name, std::wstring_view code, std::wstring_view cppCode, int expectedErrors)
{
	std::wistringstream input(std::wstring(code.data(), code.size()));
	auto lexer = LexerFactory::Create(input);
	auto parser = ParserFactory::Create(lexer->Lex());
	auto nodes = to_vector(parser->Parse());

	std::wostringstream output;
	auto codeGen = CodeGeneratorFactory::Create(output, "Cpp");
	Traverse<Node>::DepthFirstPostorder(nodes, [&](Node& node) { codeGen->Visit(node); });

	auto generatedCode = output.str();
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"^\\s+" }, L"");
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"\\s+$" }, L"");
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"\\s+" }, L" ");
	auto expectedCode = to_wstring(cppCode);
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"^\\s+" }, L"");
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"\\s+$" }, L"");
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"\\s+" }, L" ");

	bool success = generatedCode == expectedCode;

	if (!success)
	{
		std::wcerr << "ERROR: Code generator test \"" << name << "\" error: The generated code differs from the expected code.\nGenerated:\n" << generatedCode
			<< "\nExpected:\n" << expectedCode << "\n\nInput code:\n" << code << "\n\nAST:\n";

		PrintAST(nodes);

		return false;
	}

	success = codeGen->GetErrors().size() == expectedErrors;

	if (!success)
	{
		std::wcerr << "ERROR: Code generator test \"" << name << "\" error: The error count differs from the expected count.\nError messages:\n";

		for (auto& error : codeGen->GetErrors())
		{
			std::wcerr << error << '\n';
		}

		return false;
	}

	return true;
}

bool CodeGenTest::ImportFailure(const char* name, std::wstring_view code, std::wstring_view cppCode, int expectedErrors)
{
	std::wistringstream input(std::wstring(code.data(), code.size()));
	auto lexer = LexerFactory::Create(input);
	auto parser = ParserFactory::Create(lexer->Lex());
	auto nodes = to_vector(parser->Parse());

	std::wostringstream output;
	auto codeGen = CodeGeneratorFactory::Create(output, "Cpp");
	Traverse<Node>::DepthFirstPostorder(nodes, [&](Node& node) { codeGen->Visit(node); });

	bool success = codeGen->GetImports().size() == 1 || codeGen->GetImports().empty() && cppCode.empty();

	if (!success)
	{
		std::wcerr << "ERROR: Code generator import test \"" << name << "\" error: The code didn't yield a single export.\n";

		return false;
	}

	auto generatedCode = codeGen->GetImports().front();
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"^\\s+" }, L"");
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"\\s+$" }, L"");
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"\\s+" }, L" ");
	auto expectedCode = to_wstring(cppCode);
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"^\\s+" }, L"");
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"\\s+$" }, L"");
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"\\s+" }, L" ");

	success = generatedCode == expectedCode;

	if (!success)
	{
		std::wcerr << "ERROR: Code generator import test \"" << name << "\" error: The generated code differs from the expected code.\nGenerated:\n" << generatedCode
			<< "\nExpected:\n" << expectedCode << "\n\nInput code:\n" << code << "\n";

		return false;
	}

	success = codeGen->GetErrors().size() == expectedErrors;

	if (!success)
	{
		std::wcerr << "ERROR: Code generator import test \"" << name << "\" error: The error count differs from the expected count.\nError messages:\n";

		for (auto& error : codeGen->GetErrors())
		{
			std::wcerr << error << '\n';
		}

		return false;
	}

	return true;
}

bool CodeGenTest::ExportFailure(const char* name, std::wstring_view code, std::wstring_view cppCode, int expectedErrors)
{
	std::wistringstream input(std::wstring(code.data(), code.size()));
	auto lexer = LexerFactory::Create(input);
	auto parser = ParserFactory::Create(lexer->Lex());
	auto nodes = to_vector(parser->Parse());

	std::wostringstream output;
	auto codeGen = CodeGeneratorFactory::Create(output, "Cpp");
	Traverse<Node>::DepthFirstPostorder(nodes, [&](Node& node) { codeGen->Visit(node); });

	bool success = codeGen->GetExports().size() == 1 || codeGen->GetExports().empty() && cppCode.empty();

	if (!success)
	{
		std::wcerr << "ERROR: Code generator export test \"" << name << "\" error: The code didn't yield a single export.\n";

		return false;
	}

	auto generatedCode = codeGen->GetExports().front();
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"^\\s+" }, L"");
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"\\s+$" }, L"");
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"\\s+" }, L" ");
	auto expectedCode = to_wstring(cppCode);
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"^\\s+" }, L"");
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"\\s+$" }, L"");
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"\\s+" }, L" ");

	success = generatedCode == expectedCode;

	if (!success)
	{
		std::wcerr << "ERROR: Code generator export test \"" << name << "\" error: The generated code differs from the expected code.\nGenerated:\n" << generatedCode
			<< "\nExpected:\n" << expectedCode << "\n\nInput code:\n" << code << "\n";

		return false;
	}

	success = codeGen->GetErrors().size() == expectedErrors;

	if (!success)
	{
		std::wcerr << "ERROR: Code generator export test \"" << name << "\" error: The error count differs from the expected count.\nError messages:\n";

		for (auto& error : codeGen->GetErrors())
		{
			std::wcerr << error << '\n';
		}

		return false;
	}

	return true;
}

suite CodeGeneratorCpp_function_input_type_tests = [] {
	"l_i1"_test = [] { 
		expect(CodeGenTest::Success("l_i1",
			L"#1 f", 
			L"auto f_f(lovela::context& context, l_i1 in);"
		));
	};
	"l_i8"_test = [] { 
		expect(CodeGenTest::Success("l_i8", 
			L"#8 f", 
			L"auto f_f(lovela::context& context, l_i8 in);"
		));
	};
	"l_i16"_test = [] { 
		expect(CodeGenTest::Success("l_i16", 
			L"#16 f", 
			L"auto f_f(lovela::context& context, l_i16 in);"
		));
	};
	"l_i32"_test = [] { 
		expect(CodeGenTest::Success("l_i32", 
			L"#32 f", 
			L"auto f_f(lovela::context& context, l_i32 in);"
		));
	};
	"l_i64"_test = [] { 
		expect(CodeGenTest::Success("l_i64", 
			L"#64 f", 
			L"auto f_f(lovela::context& context, l_i64 in);"
		));
	};
	"l_u1"_test = [] { 
		expect(CodeGenTest::Success("l_u1", 
			L"#+1 f", 
			L"auto f_f(lovela::context& context, l_u1 in);"
		));
	};
	"l_u8"_test = [] { 
		expect(CodeGenTest::Success("l_u8", 
			L"#+8 f", 
			L"auto f_f(lovela::context& context, l_u8 in);"
		));
	};
	"l_u16"_test = [] { 
		expect(CodeGenTest::Success("l_u16", 
			L"#+16 f", 
			L"auto f_f(lovela::context& context, l_u16 in);"
		));
	};
	"l_u32"_test = [] { 
		expect(CodeGenTest::Success("l_u32", 
			L"#+32 f", 
			L"auto f_f(lovela::context& context, l_u32 in);"
		));
	};
	"l_u64"_test = [] { 
		expect(CodeGenTest::Success("l_u64", 
			L"#+64 f", 
			L"auto f_f(lovela::context& context, l_u64 in);"
		));
	};
	"l_f16 error"_test = [] { 
		expect(CodeGenTest::Failure("l_f16 error", 
			L"#.16 f", 
			L"auto f_f(lovela::context& context, t_#.16 in);", 1
		));
	};
	"l_f32"_test = [] { 
		expect(CodeGenTest::Success("l_f32", 
			L"#.32 f", 
			L"auto f_f(lovela::context& context, l_f32 in);"
		));
	};
	"l_f64"_test = [] { 
		expect(CodeGenTest::Success("l_f64", 
			L"#.64 f", 
			L"auto f_f(lovela::context& context, l_f64 in);"
		));
	};
};

suite CodeGeneratorCpp_function_declaration_tests = [] {
	"trivial function"_test = [] { 
		expect(CodeGenTest::Success("trivial function", 
			L"func",
			L"template <typename In> auto f_func(lovela::context& context, In in);"
		));
	};

	"function with return type"_test = [] { 
		expect(CodeGenTest::Success("function with return type", 
			L"func [type]",
			L"template <typename In> t_type f_func(lovela::context& context, In in);"
		));
	};

	"function with object type"_test = [] { 
		expect(CodeGenTest::Success("function with object type", 
			L"[type] func",
			L"auto f_func(lovela::context& context, t_type in);"
		));
	};

	"function with untyped parameter"_test = [] { 
		expect(CodeGenTest::Success("function with untyped parameter", 
			L"func (arg)",
			L"template <typename In, typename Param1> auto f_func(lovela::context& context, In in, Param1 p_arg);"
		));
	};

	"function with typed parameter"_test = [] { 
		expect(CodeGenTest::Success("function with typed parameter", 
			L"func (arg [type])",
			L"template <typename In> auto f_func(lovela::context& context, In in, t_type p_arg);"
		));
	};

	"trivial function"_test = [] { 
		expect(CodeGenTest::Success("trivial function", 
			L"func: + 1.", 
			LR"code(
template <typename In>
auto f_func(lovela::context& context, In in)
{ static_cast<void>(context); auto& v1 = in; static_cast<void>(v1); const auto v2 = v1 + 1; static_cast<void>(v2); return v2; }
)code"
		));
	};
};

suite CodeGeneratorCpp_function_call_tests = [] {
	"function call"_test = [] { 
		expect(CodeGenTest::Success("function call", 
			L"[#8] func [#8]: f(1, 'a', g).", 
			LR"code(
l_i8 f_func(lovela::context& context, l_i8 in)
{ static_cast<void>(context); auto& v1 = in; static_cast<void>(v1); const auto v2 = f_f(context, v1, 1, "a", f_g(context, v1)); static_cast<void>(v2); return v2; }
)code"
		));
	};
};

suite CodeGeneratorCpp_exported_functions_tests = [] {
	"exported function none -> none"_test = [] { 
		expect(CodeGenTest::Success("exported function none -> none", 
			L"<- [()] ex [()]:.", 
			LR"code(
lovela::None f_ex(lovela::context& context, lovela::None in)
{ static_cast<void>(context); auto& v1 = in; static_cast<void>(v1); return {}; }
void ex()
{ lovela::context context; lovela::None in; f_ex(context, in); }
)code"
		));
	};

	"exported function any -> any"_test = [] { 
		expect(CodeGenTest::Success("exported function any -> any", 
			L"<- ex: + 1.", 
			LR"code(
template <typename In>
auto f_ex(lovela::context& context, In in)
{ static_cast<void>(context); auto& v1 = in; static_cast<void>(v1); const auto v2 = v1 + 1; static_cast<void>(v2); return v2; }
void* ex(void* in)
{ lovela::context context; return f_ex(context, in); }
)code"
		));
	};

	"exported function C"_test = [] { 
		expect(CodeGenTest::ExportSuccess("exported function C", 
			L"<- 'C' #32 ex #32", 
			L"LOVELA_API_C l_i32 ex(l_i32 in)"
		));
	};
	"exported function C++"_test = [] { 
		expect(CodeGenTest::ExportSuccess("exported function C++", 
			L"<- 'C++' #32 ex #32", 
			L"LOVELA_API_CPP l_i32 ex(l_i32 in)"
		));
	};
	"exported function C Dynamic"_test = [] { 
		expect(CodeGenTest::ExportSuccess("exported function C Dynamic", 
			L"<- 'C Dynamic' #32 ex #32", 
			L"LOVELA_API_C LOVELA_API_DYNAMIC_EXPORT l_i32 ex(l_i32 in)"
		));
	};
};

suite CodeGeneratorCpp_imported_functions_tests = [] {
	"imported function"_test = [] { 
		expect(CodeGenTest::Success("imported function", 
			L"-> im", 
			L"template <typename In> auto f_im(lovela::context& context, In in) { static_cast<void>(context); return im(in); }"
		));
	};
	"imported function C"_test = [] { 
		expect(CodeGenTest::Success("imported function C", 
			L"-> 'C' #8 im #8", 
			L"LOVELA_API_C l_i8 im(l_i8 in); l_i8 f_im(lovela::context& context, l_i8 in) { static_cast<void>(context); return im(in); }"
		));
	};
	"imported function C Dynamic"_test = [] { 
		expect(CodeGenTest::Success("imported function C Dynamic", 
			L"-> 'C Dynamic' #8 im #8", 
			L"LOVELA_API_C LOVELA_API_DYNAMIC_IMPORT l_i8 im(l_i8 in); l_i8 f_im(lovela::context& context, l_i8 in) { static_cast<void>(context); return im(in); }"
		));
	};
	"imported function Standard C stdio"_test = [] { 
		expect(CodeGenTest::ImportSuccess("imported function Standard C stdio", 
			L"-> 'Standard C' puts", 
			L"stdio.h"
		));
	};
	"imported function Standard C stlib"_test = [] { 
		expect(CodeGenTest::ImportSuccess("imported function Standard C stlib", 
			L"-> 'Standard C' atof", 
			L"stdlib.h"
		));
	};
	"imported function Standard C string"_test = [] { 
		expect(CodeGenTest::ImportSuccess("imported function Standard C string", 
			L"-> 'Standard C' strcpy", 
			L"string.h"
		));
	};
	"imported function Standard C math"_test = [] { 
		expect(CodeGenTest::ImportSuccess("imported function Standard C math", 
			L"-> 'Standard C' sin", 
			L"math.h"
		));
	};
	"imported function Standard C++ cstdio"_test = [] { 
		expect(CodeGenTest::ImportSuccess("imported function Standard C++ cstdio", 
			L"-> 'Standard C++' std|puts", 
			L"cstdio"
		));
	};
	"imported function Standard C++ cstlib"_test = [] { 
		expect(CodeGenTest::ImportSuccess("imported function Standard C++ cstlib", 
			L"-> 'Standard C++' std|atof", 
			L"cstdlib"
		));
	};
	"imported function Standard C++ cstring"_test = [] { 
		expect(CodeGenTest::ImportSuccess("imported function Standard C++ cstring", 
			L"-> 'Standard C++' std|strcpy", 
			L"cstring"
		));
	};
	"imported function Standard C++ cmath"_test = [] { 
		expect(CodeGenTest::ImportSuccess("imported function Standard C++ cmath", 
			L"-> 'Standard C++' std|sin", 
			L"cmath"
		));
	};
};

suite CodeGeneratorCpp_main_function_tests = [] {
	"main and export"_test = [] { 
		expect(CodeGenTest::Success("main and export", 
			L"<- [#32] ex [#32]: + 1. : 1 ex.", 
			LR"code(
l_i32 f_ex(lovela::context& context, l_i32 in)
{
	static_cast<void>(context);
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
	static_cast<void>(context);
	auto& v1 = in; static_cast<void>(v1);
	const auto v2 = f_ex(context, 1); static_cast<void>(v2);
	return {};
}
)code"
		));
	};

	"main and explicitly typed import"_test = [] { 
		expect(CodeGenTest::Success("main and explicitly typed import", 
			L"-> [#8#] puts [#32]. : 'Hello, Wordl!' puts.", 
			LR"code(
l_i32 f_puts(lovela::context& context, l_cstr in)
{
        static_cast<void>(context);
        return puts(in);
}

lovela::None lovela::main(lovela::context& context, lovela::None in)
{
        static_cast<void>(context);
        auto& v1 = in; static_cast<void>(v1);
        const auto v2 = f_puts(context, "Hello, Wordl!"); static_cast<void>(v2);
        return {};
}
)code"
		));
	};

	"main and implicitly typed import"_test = [] { 
		expect(CodeGenTest::Success("main and implicitly typed import", 
			L"-> puts. : 'Hello, Wordl!' puts.", 
			LR"code(
template <typename In>
auto f_puts(lovela::context& context, In in)
{
	static_cast<void>(context);
	return puts(in);
}

lovela::None lovela::main(lovela::context& context, lovela::None in)
{
	static_cast<void>(context);
	auto& v1 = in; static_cast<void>(v1);
	const auto v2 = f_puts(context, "Hello, Wordl!"); static_cast<void>(v2);
	return {};
}
)code"
		));
	};
};
