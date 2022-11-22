#include "pch.h"
#include "TestingBase.h"
#include "../lovela/LexerFactory.h"
#include "../lovela/ParserFactory.h"
#include "../lovela/CodeGeneratorFactory.h"

#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp"
using namespace boost::ut;

class CodeGeneratorCppTest : public TestingBase
{
public:
	bool Success(const char* name, std::string_view code, std::string_view cppCode)
	{
		return Failure(name, code, cppCode, 0);
	}

	bool ImportSuccess(const char* name, std::string_view code, std::string_view cppCode)
	{
		return ImportFailure(name, code, cppCode, 0);
	}

	bool ExportSuccess(const char* name, std::string_view code, std::string_view cppCode)
	{
		return ExportFailure(name, code, cppCode, 0);
	}

	bool Failure(const char* name, std::string_view code, std::string_view cppCode, int expectedErrors);
	bool ImportFailure(const char* name, std::string_view code, std::string_view cppCode, int expectedErrors);
	bool ExportFailure(const char* name, std::string_view code, std::string_view cppCode, int expectedErrors);
};

static CodeGeneratorCppTest s_test;

bool CodeGeneratorCppTest::Failure(const char* name, std::string_view code, std::string_view cppCode, int expectedErrors)
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
		std::cerr << color.fail << "ERROR: " << color.none << "Code generator test \"" << name << "\" error: The generated code differs from the expected code.\nGenerated:\n" << generatedCode
			<< "\nExpected:\n" << expectedCode << "\n\nInput code:\n" << code << "\n\nAST:\n";

		PrintAST(nodes);

		return false;
	}

	success = codeGen->GetErrors().size() == expectedErrors;

	if (!success)
	{
		std::cerr << color.fail << "ERROR: " << color.none << "Code generator test \"" << name << "\" error: The error count differs from the expected count.\nError messages:\n";

		for (auto& error : codeGen->GetErrors())
		{
			std::cerr << error << '\n';
		}

		return false;
	}

	return true;
}

bool CodeGeneratorCppTest::ImportFailure(const char* name, std::string_view code, std::string_view cppCode, int expectedErrors)
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
		std::cerr << color.fail << "ERROR: " << color.none << "Code generator import test \"" << name << "\" error: The code didn't yield a single export.\n";

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
		std::cerr << color.fail << "ERROR: " << color.none << "Code generator import test \"" << name << "\" error: The generated code differs from the expected code.\nGenerated:\n" << generatedCode
			<< "\nExpected:\n" << expectedCode << "\n\nInput code:\n" << code << "\n";

		return false;
	}

	success = codeGen->GetErrors().size() == expectedErrors;

	if (!success)
	{
		std::cerr << color.fail << "ERROR: " << color.none << "Code generator import test \"" << name << "\" error: The error count differs from the expected count.\nError messages:\n";

		for (auto& error : codeGen->GetErrors())
		{
			std::cerr << error << '\n';
		}

		return false;
	}

	return true;
}

bool CodeGeneratorCppTest::ExportFailure(const char* name, std::string_view code, std::string_view cppCode, int expectedErrors)
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
		std::cerr << color.fail << "ERROR: " << color.none << "Code generator export test \"" << name << "\" error: The code didn't yield a single export.\n";

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
		std::cerr << color.fail << "ERROR: " << color.none << "Code generator export test \"" << name << "\" error: The generated code differs from the expected code.\nGenerated:\n" << generatedCode
			<< "\nExpected:\n" << expectedCode << "\n\nInput code:\n" << code << "\n";

		return false;
	}

	success = codeGen->GetErrors().size() == expectedErrors;

	if (!success)
	{
		std::cerr << color.fail << "ERROR: " << color.none << "Code generator export test \"" << name << "\" error: The error count differs from the expected count.\nError messages:\n";

		for (auto& error : codeGen->GetErrors())
		{
			std::cerr << error << '\n';
		}

		return false;
	}

	return true;
}

suite CodeGeneratorCpp_function_input_type_tests = [] {
	"l_i1 input"_test = [] { 
		expect(s_test.Success("l_i1 input",
			"#1 f", 
			R"cpp(auto f_f(lovela::context& context, l_i1 in);)cpp"
		));
	};

	"l_i8 input"_test = [] { 
		expect(s_test.Success("l_i8 input", 
			"#8 f", 
			R"cpp(auto f_f(lovela::context& context, l_i8 in);)cpp"
		));
	};

	"l_i16 input"_test = [] { 
		expect(s_test.Success("l_i16 input", 
			"#16 f", 
			R"cpp(auto f_f(lovela::context& context, l_i16 in);)cpp"
		));
	};

	"l_i32 input"_test = [] { 
		expect(s_test.Success("l_i32 input", 
			"#32 f", 
			R"cpp(auto f_f(lovela::context& context, l_i32 in);)cpp"
		));
	};

	"l_i64 input"_test = [] { 
		expect(s_test.Success("l_i64 input", 
			"#64 f", 
			R"cpp(auto f_f(lovela::context& context, l_i64 in);)cpp"
		));
	};

	"l_i2 input error"_test = [] {
		expect(s_test.Failure("l_i2 input error",
			"#2 f",
			R"cpp(auto f_f(lovela::context& context, InvalidTypeName in);)cpp", 1
		));
	};

	"l_u1 input"_test = [] { 
		expect(s_test.Success("l_u1 input", 
			"#+1 f", 
			R"cpp(auto f_f(lovela::context& context, l_u1 in);)cpp"
		));
	};

	"l_u8 input"_test = [] { 
		expect(s_test.Success("l_u8 input", 
			"#+8 f", 
			R"cpp(auto f_f(lovela::context& context, l_u8 in);)cpp"
		));
	};

	"l_u16 input"_test = [] { 
		expect(s_test.Success("l_u16 input", 
			"#+16 f", 
			R"cpp(auto f_f(lovela::context& context, l_u16 in);)cpp"
		));
	};

	"l_u32 input"_test = [] { 
		expect(s_test.Success("l_u32 input", 
			"#+32 f", 
			R"cpp(auto f_f(lovela::context& context, l_u32 in);)cpp"
		));
	};

	"l_u64 input"_test = [] { 
		expect(s_test.Success("l_u64 input", 
			"#+64 f", 
			R"cpp(auto f_f(lovela::context& context, l_u64 in);)cpp"
		));
	};

	"l_f16 input error"_test = [] { 
		expect(s_test.Failure("l_f16 input error", 
			"#.16 f", 
			R"cpp(auto f_f(lovela::context& context, InvalidTypeName in);)cpp", 1
		));
	};

	"l_f32 input"_test = [] { 
		expect(s_test.Success("l_f32 input", 
			"#.32 f", 
			R"cpp(auto f_f(lovela::context& context, l_f32 in);)cpp"
		));
	};

	"l_f64 input"_test = [] { 
		expect(s_test.Success("l_f64 input", 
			"#.64 f", 
			R"cpp(auto f_f(lovela::context& context, l_f64 in);)cpp"
		));
	};

	"[1] input"_test = [] {
		expect(s_test.Success("[1] input",
			"[1] f",
			R"cpp(
template <typename Tag1>
auto f_f(lovela::context& context, Tag1 in);)cpp"
		));
	};
};

suite CodeGeneratorCpp_function_output_type_tests = [] {
	"l_i1 output"_test = [] {
		expect(s_test.Success("l_i1 output",
			"f #1",
			R"cpp(l_i1 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_i8 output"_test = [] {
		expect(s_test.Success("l_i8 output",
			"f #8",
			R"cpp(l_i8 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_i16 output"_test = [] {
		expect(s_test.Success("l_i16 output",
			"f #16",
			R"cpp(l_i16 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_i32 output"_test = [] {
		expect(s_test.Success("l_i32 output",
			"f #32",
			R"cpp(l_i32 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_i64 output"_test = [] {
		expect(s_test.Success("l_i64 output",
			"f #64",
			R"cpp(l_i64 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_i2 output error"_test = [] {
		expect(s_test.Failure("l_i2 output error",
			"f #2",
			R"cpp(InvalidTypeName f_f(lovela::context& context, auto in);)cpp", 1
		));
	};

	"l_u1 output"_test = [] {
		expect(s_test.Success("l_u1 output",
			"f #+1",
			R"cpp(l_u1 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_u8 output"_test = [] {
		expect(s_test.Success("l_u8 output",
			"f #+8",
			R"cpp(l_u8 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_u16 output"_test = [] {
		expect(s_test.Success("l_u16 output",
			"f #+16",
			R"cpp(l_u16 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_u32 output"_test = [] {
		expect(s_test.Success("l_u32 output",
			"f #+32",
			R"cpp(l_u32 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_u64 output"_test = [] {
		expect(s_test.Success("l_u64 output",
			"f #+64",
			R"cpp(l_u64 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_f16 output error"_test = [] {
		expect(s_test.Failure("l_f16 output error",
			"f #.16",
			R"cpp(InvalidTypeName f_f(lovela::context& context, auto in);)cpp", 1
		));
	};

	"l_f32 output"_test = [] {
		expect(s_test.Success("l_f32 output",
			"f #.32",
			R"cpp(l_f32 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_f64 output"_test = [] {
		expect(s_test.Success("l_f64 output",
			"f #.64",
			R"cpp(l_f64 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"[1] output"_test = [] {
		expect(s_test.Success("[1] output",
			"f [1]",
			R"cpp(
template <typename Tag1>
Tag1 f_f(lovela::context& context, auto in);)cpp"
));
	};
};

suite CodeGeneratorCpp_function_param_type_tests = [] {
	"l_i1 param"_test = [] {
		expect(s_test.Success("l_i1 param",
			"f (#1)",
			R"cpp(auto f_f(lovela::context& context, auto in, l_i1 param1);)cpp"
		));
	};

	"l_i32 param"_test = [] {
		expect(s_test.Success("l_i32 param",
			"f (#32)",
			R"cpp(auto f_f(lovela::context& context, auto in, l_i32 param1);)cpp"
		));
	};

	"l_i2 param error"_test = [] {
		expect(s_test.Failure("l_i2 param error",
			"f (#2)",
			R"cpp(auto f_f(lovela::context& context, auto in, InvalidTypeName param1);)cpp", 1
		));
	};

	"l_u8 param"_test = [] {
		expect(s_test.Success("l_u8 param",
			"f (#+8)",
			R"cpp(auto f_f(lovela::context& context, auto in, l_u8 param1);)cpp"
		));
	};

	"l_u64 param"_test = [] {
		expect(s_test.Success("l_u64 param",
			"f (#+64)",
			R"cpp(auto f_f(lovela::context& context, auto in, l_u64 param1);)cpp"
		));
	};

	"l_f16 param error"_test = [] {
		expect(s_test.Failure("l_f16 param error",
			"f (#.16)",
			R"cpp(auto f_f(lovela::context& context, auto in, InvalidTypeName param1);)cpp", 1
		));
	};

	"l_f32 param"_test = [] {
		expect(s_test.Success("l_f32 param",
			"f (#.32)",
			R"cpp(auto f_f(lovela::context& context, auto in, l_f32 param1);)cpp"
		));
	};

	"[1] param"_test = [] {
		expect(s_test.Success("[1] param",
			"f ([1])",
			R"cpp(
template <typename Tag1>
auto f_f(lovela::context& context, auto in, Tag1 param1);)cpp"
));
	};
};

suite CodeGeneratorCpp_function_declaration_tests = [] {
	"trivial function"_test = [] { 
		expect(s_test.Success("trivial function", 
			"func",
			R"cpp(
auto f_func(lovela::context& context, auto in);
)cpp"
		));
	};

	"function with return type"_test = [] { 
		expect(s_test.Success("function with return type", 
			"func [type]",
			R"cpp(
t_type f_func(lovela::context& context, auto in);
)cpp"
		));
	};

	"function with object type"_test = [] { 
		expect(s_test.Success("function with object type", 
			"[type] func",
			R"cpp(
auto f_func(lovela::context& context, t_type in);
)cpp"
		));
	};

	"function with untyped parameter"_test = [] { 
		expect(s_test.Success("function with untyped parameter", 
			"func (arg)",
			R"cpp(
auto f_func(lovela::context& context, auto in, auto p_arg);
)cpp"
		));
	};

	"function with typed parameter"_test = [] { 
		expect(s_test.Success("function with typed parameter", 
			"func (arg [type])",
			R"cpp(
auto f_func(lovela::context& context, auto in, t_type p_arg);
)cpp"
		));
	};

	"trivial function"_test = [] { 
		expect(s_test.Success("trivial function", 
			"func: + 1.", 
			R"cpp(
auto f_func(lovela::context& context, auto in)
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
		expect(s_test.Success("function call", 
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
		expect(s_test.Success("exported function none -> none",
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
		expect(s_test.Success("exported function any -> any",
			"<- ex: + 1.",
			R"cpp(
auto f_ex(lovela::context& context, auto in)
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
		expect(s_test.ExportSuccess("exported function C", 
			"<- 'C' #32 ex #32", 
			"LOVELA_API_C l_i32 ex(l_i32 in)"
		));
	};

	"exported function C++"_test = [] { 
		expect(s_test.ExportSuccess("exported function C++", 
			"<- 'C++' #32 ex #32", 
			"LOVELA_API_CPP l_i32 ex(l_i32 in)"
		));
	};

	"exported function C Dynamic"_test = [] { 
		expect(s_test.ExportSuccess("exported function C Dynamic", 
			"<- 'C Dynamic' #32 ex #32", 
			"LOVELA_API_C LOVELA_API_DYNAMIC_EXPORT l_i32 ex(l_i32 in)"
		));
	};
};

suite CodeGeneratorCpp_imported_functions_tests = [] {
	"imported function"_test = [] {
		expect(s_test.Success("imported function",
			"-> im",
			R"cpp(
auto f_im(lovela::context& context, auto in)
{
	static_cast<void>(context); return im(in);
}
)cpp"
));
	};

	"imported function C"_test = [] {
		expect(s_test.Success("imported function C",
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
		expect(s_test.Success("imported function C Dynamic",
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
		expect(s_test.ImportSuccess("imported function Standard C stdio", 
			"-> 'Standard C' puts", 
			"stdio.h"
		));
	};

	"imported function Standard C stlib"_test = [] { 
		expect(s_test.ImportSuccess("imported function Standard C stlib", 
			"-> 'Standard C' atof", 
			"stdlib.h"
		));
	};

	"imported function Standard C string"_test = [] { 
		expect(s_test.ImportSuccess("imported function Standard C string", 
			"-> 'Standard C' strcpy", 
			"string.h"
		));
	};

	"imported function Standard C math"_test = [] { 
		expect(s_test.ImportSuccess("imported function Standard C math", 
			"-> 'Standard C' sin", 
			"math.h"
		));
	};

	"imported function Standard C++ cstdio"_test = [] { 
		expect(s_test.ImportSuccess("imported function Standard C++ cstdio", 
			"-> 'Standard C++' std|puts", 
			"cstdio"
		));
	};

	"imported function Standard C++ cstlib"_test = [] { 
		expect(s_test.ImportSuccess("imported function Standard C++ cstlib", 
			"-> 'Standard C++' std|atof", 
			"cstdlib"
		));
	};

	"imported function Standard C++ cstring"_test = [] { 
		expect(s_test.ImportSuccess("imported function Standard C++ cstring", 
			"-> 'Standard C++' std|strcpy", 
			"cstring"
		));
	};

	"imported function Standard C++ cmath"_test = [] { 
		expect(s_test.ImportSuccess("imported function Standard C++ cmath", 
			"-> 'Standard C++' std|sin", 
			"cmath"
		));
	};
};

suite CodeGeneratorCpp_main_function_tests = [] {
	"main and export"_test = [] { 
		expect(s_test.Success("main and export", 
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
		expect(s_test.Success("main and explicitly typed import", 
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
		expect(s_test.Success("main and implicitly typed import", 
			"-> puts. : 'Hello, Wordl!' puts.", 
			R"cpp(
auto f_puts(lovela::context& context, auto in)
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
