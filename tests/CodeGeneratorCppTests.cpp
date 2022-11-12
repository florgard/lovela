#include "pch.h"
#include "TestingBase.h"
#include "../lovela/LexerFactory.h"
#include "../lovela/ParserFactory.h"
#include "../lovela/CodeGeneratorFactory.h"

#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp"
using namespace boost::ut;

class CodeGenTest : public TestingBase
{
public:
	static bool Success(const char* name, std::string_view code, std::string_view cppCode)
	{
		return Failure(name, code, cppCode, 0);
	}

	static bool ImportSuccess(const char* name, std::string_view code, std::string_view cppCode)
	{
		return ImportFailure(name, code, cppCode, 0);
	}

	static bool ExportSuccess(const char* name, std::string_view code, std::string_view cppCode)
	{
		return ExportFailure(name, code, cppCode, 0);
	}

	static bool Failure(const char* name, std::string_view code, std::string_view cppCode, int expectedErrors);
	static bool ImportFailure(const char* name, std::string_view code, std::string_view cppCode, int expectedErrors);
	static bool ExportFailure(const char* name, std::string_view code, std::string_view cppCode, int expectedErrors);
};

bool CodeGenTest::Failure(const char* name, std::string_view code, std::string_view cppCode, int expectedErrors)
{
	std::istringstream input(std::string(code.data(), code.size()));
	auto lexer = LexerFactory::Create(input);
	auto parser = ParserFactory::Create(lexer->Lex());
	auto nodes = to_vector(parser->Parse());

	std::ostringstream output;
	auto codeGen = CodeGeneratorFactory::Create(output, "Cpp");
	Traverse<Node>::DepthFirstPostorder(nodes, [&](Node& node) { codeGen->Visit(node); });

	auto generatedCode = output.str();
	generatedCode = std::regex_replace(generatedCode, std::regex{ "^\\s+" }, "");
	generatedCode = std::regex_replace(generatedCode, std::regex{ "\\s+$" }, "");
	generatedCode = std::regex_replace(generatedCode, std::regex{ "\\s+" }, " ");
	auto expectedCode = to_string(cppCode);
	expectedCode = std::regex_replace(expectedCode, std::regex{ "^\\s+" }, "");
	expectedCode = std::regex_replace(expectedCode, std::regex{ "\\s+$" }, "");
	expectedCode = std::regex_replace(expectedCode, std::regex{ "\\s+" }, " ");

	bool success = generatedCode == expectedCode;

	if (!success)
	{
		std::cerr << "ERROR: Code generator test \"" << name << "\" error: The generated code differs from the expected code.\nGenerated:\n" << generatedCode
			<< "\nExpected:\n" << expectedCode << "\n\nInput code:\n" << code << "\n\nAST:\n";

		PrintAST(nodes);

		return false;
	}

	success = codeGen->GetErrors().size() == expectedErrors;

	if (!success)
	{
		std::cerr << "ERROR: Code generator test \"" << name << "\" error: The error count differs from the expected count.\nError messages:\n";

		for (auto& error : codeGen->GetErrors())
		{
			std::cerr << error << '\n';
		}

		return false;
	}

	return true;
}

bool CodeGenTest::ImportFailure(const char* name, std::string_view code, std::string_view cppCode, int expectedErrors)
{
	std::istringstream input(std::string(code.data(), code.size()));
	auto lexer = LexerFactory::Create(input);
	auto parser = ParserFactory::Create(lexer->Lex());
	auto nodes = to_vector(parser->Parse());

	std::ostringstream output;
	auto codeGen = CodeGeneratorFactory::Create(output, "Cpp");
	Traverse<Node>::DepthFirstPostorder(nodes, [&](Node& node) { codeGen->Visit(node); });

	bool success = codeGen->GetImports().size() == 1 || codeGen->GetImports().empty() && cppCode.empty();

	if (!success)
	{
		std::cerr << "ERROR: Code generator import test \"" << name << "\" error: The code didn't yield a single export.\n";

		return false;
	}

	auto generatedCode = codeGen->GetImports().front();
	generatedCode = std::regex_replace(generatedCode, std::regex{ "^\\s+" }, "");
	generatedCode = std::regex_replace(generatedCode, std::regex{ "\\s+$" }, "");
	generatedCode = std::regex_replace(generatedCode, std::regex{ "\\s+" }, " ");
	auto expectedCode = to_string(cppCode);
	expectedCode = std::regex_replace(expectedCode, std::regex{ "^\\s+" }, "");
	expectedCode = std::regex_replace(expectedCode, std::regex{ "\\s+$" }, "");
	expectedCode = std::regex_replace(expectedCode, std::regex{ "\\s+" }, " ");

	success = generatedCode == expectedCode;

	if (!success)
	{
		std::cerr << "ERROR: Code generator import test \"" << name << "\" error: The generated code differs from the expected code.\nGenerated:\n" << generatedCode
			<< "\nExpected:\n" << expectedCode << "\n\nInput code:\n" << code << "\n";

		return false;
	}

	success = codeGen->GetErrors().size() == expectedErrors;

	if (!success)
	{
		std::cerr << "ERROR: Code generator import test \"" << name << "\" error: The error count differs from the expected count.\nError messages:\n";

		for (auto& error : codeGen->GetErrors())
		{
			std::cerr << error << '\n';
		}

		return false;
	}

	return true;
}

bool CodeGenTest::ExportFailure(const char* name, std::string_view code, std::string_view cppCode, int expectedErrors)
{
	std::istringstream input(std::string(code.data(), code.size()));
	auto lexer = LexerFactory::Create(input);
	auto parser = ParserFactory::Create(lexer->Lex());
	auto nodes = to_vector(parser->Parse());

	std::ostringstream output;
	auto codeGen = CodeGeneratorFactory::Create(output, "Cpp");
	Traverse<Node>::DepthFirstPostorder(nodes, [&](Node& node) { codeGen->Visit(node); });

	bool success = codeGen->GetExports().size() == 1 || codeGen->GetExports().empty() && cppCode.empty();

	if (!success)
	{
		std::cerr << "ERROR: Code generator export test \"" << name << "\" error: The code didn't yield a single export.\n";

		return false;
	}

	auto generatedCode = codeGen->GetExports().front();
	generatedCode = std::regex_replace(generatedCode, std::regex{ "^\\s+" }, "");
	generatedCode = std::regex_replace(generatedCode, std::regex{ "\\s+$" }, "");
	generatedCode = std::regex_replace(generatedCode, std::regex{ "\\s+" }, " ");
	auto expectedCode = to_string(cppCode);
	expectedCode = std::regex_replace(expectedCode, std::regex{ "^\\s+" }, "");
	expectedCode = std::regex_replace(expectedCode, std::regex{ "\\s+$" }, "");
	expectedCode = std::regex_replace(expectedCode, std::regex{ "\\s+" }, " ");

	success = generatedCode == expectedCode;

	if (!success)
	{
		std::cerr << "ERROR: Code generator export test \"" << name << "\" error: The generated code differs from the expected code.\nGenerated:\n" << generatedCode
			<< "\nExpected:\n" << expectedCode << "\n\nInput code:\n" << code << "\n";

		return false;
	}

	success = codeGen->GetErrors().size() == expectedErrors;

	if (!success)
	{
		std::cerr << "ERROR: Code generator export test \"" << name << "\" error: The error count differs from the expected count.\nError messages:\n";

		for (auto& error : codeGen->GetErrors())
		{
			std::cerr << error << '\n';
		}

		return false;
	}

	return true;
}

suite CodeGeneratorCpp_function_input_type_tests = [] {
	"l_i1"_test = [] { 
		expect(CodeGenTest::Success("l_i1",
			"#1 f", 
			R"cpp(auto f_f(lovela::context& context, l_i1 in);)cpp"
		));
	};

	"l_i8"_test = [] { 
		expect(CodeGenTest::Success("l_i8", 
			"#8 f", 
			R"cpp(auto f_f(lovela::context& context, l_i8 in);)cpp"
		));
	};

	"l_i16"_test = [] { 
		expect(CodeGenTest::Success("l_i16", 
			"#16 f", 
			R"cpp(auto f_f(lovela::context& context, l_i16 in);)cpp"
		));
	};

	"l_i32"_test = [] { 
		expect(CodeGenTest::Success("l_i32", 
			"#32 f", 
			R"cpp(auto f_f(lovela::context& context, l_i32 in);)cpp"
		));
	};

	"l_i64"_test = [] { 
		expect(CodeGenTest::Success("l_i64", 
			"#64 f", 
			R"cpp(auto f_f(lovela::context& context, l_i64 in);)cpp"
		));
	};

	"l_u1"_test = [] { 
		expect(CodeGenTest::Success("l_u1", 
			"#+1 f", 
			R"cpp(auto f_f(lovela::context& context, l_u1 in);)cpp"
		));
	};

	"l_u8"_test = [] { 
		expect(CodeGenTest::Success("l_u8", 
			"#+8 f", 
			R"cpp(auto f_f(lovela::context& context, l_u8 in);)cpp"
		));
	};

	"l_u16"_test = [] { 
		expect(CodeGenTest::Success("l_u16", 
			"#+16 f", 
			R"cpp(auto f_f(lovela::context& context, l_u16 in);)cpp"
		));
	};

	"l_u32"_test = [] { 
		expect(CodeGenTest::Success("l_u32", 
			"#+32 f", 
			R"cpp(auto f_f(lovela::context& context, l_u32 in);)cpp"
		));
	};

	"l_u64"_test = [] { 
		expect(CodeGenTest::Success("l_u64", 
			"#+64 f", 
			R"cpp(auto f_f(lovela::context& context, l_u64 in);)cpp"
		));
	};

	"l_f16 error"_test = [] { 
		expect(CodeGenTest::Failure("l_f16 error", 
			"#.16 f", 
			R"cpp(auto f_f(lovela::context& context, t_#.16 in);)cpp", 1
		));
	};

	"l_f32"_test = [] { 
		expect(CodeGenTest::Success("l_f32", 
			"#.32 f", 
			R"cpp(auto f_f(lovela::context& context, l_f32 in);)cpp"
		));
	};

	"l_f64"_test = [] { 
		expect(CodeGenTest::Success("l_f64", 
			"#.64 f", 
			R"cpp(auto f_f(lovela::context& context, l_f64 in);)cpp"
		));
	};
};

suite CodeGeneratorCpp_function_declaration_tests = [] {
	"trivial function"_test = [] { 
		expect(CodeGenTest::Success("trivial function", 
			"func",
			R"cpp(
template <typename In>
auto f_func(lovela::context& context, In in);
)cpp"
		));
	};

	"function with return type"_test = [] { 
		expect(CodeGenTest::Success("function with return type", 
			"func [type]",
			R"cpp(
template <typename In>
t_type f_func(lovela::context& context, In in);
)cpp"
		));
	};

	"function with object type"_test = [] { 
		expect(CodeGenTest::Success("function with object type", 
			"[type] func",
			R"cpp(
auto f_func(lovela::context& context, t_type in);
)cpp"
		));
	};

	"function with untyped parameter"_test = [] { 
		expect(CodeGenTest::Success("function with untyped parameter", 
			"func (arg)",
			R"cpp(
template <typename In, typename Param1>
auto f_func(lovela::context& context, In in, Param1 p_arg);
)cpp"
		));
	};

	"function with typed parameter"_test = [] { 
		expect(CodeGenTest::Success("function with typed parameter", 
			"func (arg [type])",
			R"cpp(
template <typename In>
auto f_func(lovela::context& context, In in, t_type p_arg);
)cpp"
		));
	};

	"trivial function"_test = [] { 
		expect(CodeGenTest::Success("trivial function", 
			"func: + 1.", 
			R"cpp(
template <typename In>
auto f_func(lovela::context& context, In in)
{
	static_cast<void>(context);
	auto& v1 = in; static_cast<void>(v1);
	const auto v2 = v1 + 1; static_cast<void>(v2);
	return v2;
}
)cpp"
		));
	};
};

suite CodeGeneratorCpp_function_call_tests = [] {
	"function call"_test = [] { 
		expect(CodeGenTest::Success("function call", 
			"[#8] func [#8]: f(1, 'a', g).", 
			R"cpp(
l_i8 f_func(lovela::context& context, l_i8 in)
{
	static_cast<void>(context);
	auto& v1 = in; static_cast<void>(v1);
	const auto v2 = f_f(context, v1, 1, "a", f_g(context, v1)); static_cast<void>(v2);
	return v2;
}
)cpp"
		));
	};
};

suite CodeGeneratorCpp_exported_functions_with_implementation_tests = [] {
	"exported function none -> none"_test = [] {
		expect(CodeGenTest::Success("exported function none -> none",
			"<- [()] ex [()]:.",
			R"cpp(
lovela::None f_ex(lovela::context& context, lovela::None in)
{
	static_cast<void>(context);
	auto& v1 = in; static_cast<void>(v1);
	return {};
}

void ex()
{
	lovela::context context;
	lovela::None in;
	f_ex(context, in);
}
)cpp"
));
	};

	"exported function any -> any"_test = [] {
		expect(CodeGenTest::Success("exported function any -> any",
			"<- ex: + 1.",
			R"cpp(
template <typename In>
auto f_ex(lovela::context& context, In in)
{
	static_cast<void>(context);
	auto& v1 = in; static_cast<void>(v1);
	const auto v2 = v1 + 1; static_cast<void>(v2);
	return v2;
}

void* ex(void* in)
{
	lovela::context context;
	return f_ex(context, in);
}
)cpp"
));
	};
};

suite CodeGeneratorCpp_exported_functions_tests = [] {
	"exported function C"_test = [] { 
		expect(CodeGenTest::ExportSuccess("exported function C", 
			"<- 'C' #32 ex #32", 
			"LOVELA_API_C l_i32 ex(l_i32 in)"
		));
	};

	"exported function C++"_test = [] { 
		expect(CodeGenTest::ExportSuccess("exported function C++", 
			"<- 'C++' #32 ex #32", 
			"LOVELA_API_CPP l_i32 ex(l_i32 in)"
		));
	};

	"exported function C Dynamic"_test = [] { 
		expect(CodeGenTest::ExportSuccess("exported function C Dynamic", 
			"<- 'C Dynamic' #32 ex #32", 
			"LOVELA_API_C LOVELA_API_DYNAMIC_EXPORT l_i32 ex(l_i32 in)"
		));
	};
};

suite CodeGeneratorCpp_imported_functions_tests = [] {
	"imported function"_test = [] {
		expect(CodeGenTest::Success("imported function",
			"-> im",
			R"cpp(
template <typename In>
auto f_im(lovela::context& context, In in)
{
	static_cast<void>(context); return im(in);
}
)cpp"
));
	};

	"imported function C"_test = [] {
		expect(CodeGenTest::Success("imported function C",
			"-> 'C' #8 im #8",
			R"cpp(
LOVELA_API_C l_i8 im(l_i8 in);

l_i8 f_im(lovela::context& context, l_i8 in)
{
	static_cast<void>(context); return im(in);
}
)cpp"
));
	};

	"imported function C Dynamic"_test = [] {
		expect(CodeGenTest::Success("imported function C Dynamic",
			"-> 'C Dynamic' #8 im #8",
			R"cpp(
LOVELA_API_C LOVELA_API_DYNAMIC_IMPORT l_i8 im(l_i8 in);

l_i8 f_im(lovela::context& context, l_i8 in)
{
	static_cast<void>(context); return im(in);
}
)cpp"
));
	};
};

suite CodeGeneratorCpp_imported_standard_functions_tests = [] {
	"imported function Standard C stdio"_test = [] { 
		expect(CodeGenTest::ImportSuccess("imported function Standard C stdio", 
			"-> 'Standard C' puts", 
			"stdio.h"
		));
	};

	"imported function Standard C stlib"_test = [] { 
		expect(CodeGenTest::ImportSuccess("imported function Standard C stlib", 
			"-> 'Standard C' atof", 
			"stdlib.h"
		));
	};

	"imported function Standard C string"_test = [] { 
		expect(CodeGenTest::ImportSuccess("imported function Standard C string", 
			"-> 'Standard C' strcpy", 
			"string.h"
		));
	};

	"imported function Standard C math"_test = [] { 
		expect(CodeGenTest::ImportSuccess("imported function Standard C math", 
			"-> 'Standard C' sin", 
			"math.h"
		));
	};

	"imported function Standard C++ cstdio"_test = [] { 
		expect(CodeGenTest::ImportSuccess("imported function Standard C++ cstdio", 
			"-> 'Standard C++' std|puts", 
			"cstdio"
		));
	};

	"imported function Standard C++ cstlib"_test = [] { 
		expect(CodeGenTest::ImportSuccess("imported function Standard C++ cstlib", 
			"-> 'Standard C++' std|atof", 
			"cstdlib"
		));
	};

	"imported function Standard C++ cstring"_test = [] { 
		expect(CodeGenTest::ImportSuccess("imported function Standard C++ cstring", 
			"-> 'Standard C++' std|strcpy", 
			"cstring"
		));
	};

	"imported function Standard C++ cmath"_test = [] { 
		expect(CodeGenTest::ImportSuccess("imported function Standard C++ cmath", 
			"-> 'Standard C++' std|sin", 
			"cmath"
		));
	};
};

suite CodeGeneratorCpp_main_function_tests = [] {
	"main and export"_test = [] { 
		expect(CodeGenTest::Success("main and export", 
			"<- [#32] ex [#32]: + 1. : 1 ex.", 
			R"cpp(
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
)cpp"
		));
	};

	"main and explicitly typed import"_test = [] { 
		expect(CodeGenTest::Success("main and explicitly typed import", 
			"-> [#8#] puts [#32]. : 'Hello, Wordl!' puts.", 
			R"cpp(
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
)cpp"
		));
	};

	"main and implicitly typed import"_test = [] { 
		expect(CodeGenTest::Success("main and implicitly typed import", 
			"-> puts. : 'Hello, Wordl!' puts.", 
			R"cpp(
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
)cpp"
		));
	};
};
