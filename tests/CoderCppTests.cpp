#include "pch.h"
#include "TestingBase.h"
#include "../lovela/Lexer.h"
#include "../lovela/Parser.h"
#include "../lovela/CoderCpp.h"

class CoderCppTest : public TestingBase
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

static CoderCppTest s_test;

bool CoderCppTest::Failure(const char* name, std::string_view code, std::string_view cppCode, int expectedErrors)
{
	StringLexer lexer;
	RangeParser<> parser;
	std::vector<Node> nodes;
	CoderCpp coder;
	std::ostringstream output;
	code >> lexer >> parser >> nodes >> coder >> output;

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
		std::cerr << color.fail << "ERROR: " << color.none
			<< "Code generator test \"" << color.name << name << color.none << "\": "
			<< "The generated code differs from the expected code.\n"
			<< "Generated:\n" << color.actual << generatedCode << color.none << '\n'
			<< "Expected:\n" << color.expect << expectedCode << color.none << '\n'
			<< "Input code:\n" << color.code << code << color.none << '\n'
			<< "AST:\n" << color.actual;
		PrintAST(nodes);
		std::cerr << color.none << '\n';

		return false;
	}

	const auto actualErrorCount = coder.GetErrors().size();
	success = actualErrorCount == expectedErrors;

	if (!success)
	{
		std::cerr << color.fail << "ERROR: " << color.none
			<< "Code generator test \"" << color.name << name << color.none << "\": "
			<< "The actual error count " << actualErrorCount << " differs from the expected count " << expectedErrors << ".\n"
			<< "Error messages:\n";

		for (auto& error : coder.GetErrors())
		{
			std::cerr << error << '\n';
		}

		return false;
	}

	return true;
}

bool CoderCppTest::ImportFailure(const char* name, std::string_view code, std::string_view cppCode, int expectedErrors)
{
	StringLexer lexer;
	RangeParser<> parser;
	std::vector<Node> nodes;
	CoderCpp coder;
	std::ostringstream output;
	code >> lexer >> parser >> nodes >> coder >> output;

	bool success = coder.GetImports().size() == 1 || coder.GetImports().empty() && cppCode.empty();

	if (!success)
	{
		std::cerr << color.fail << "ERROR: " << color.none
			<< "Code generator import test \"" << color.name << name << color.none << "\": "
			<< "The code didn't yield a single export.\n";

		return false;
	}

	auto generatedCode = coder.GetImports().front();
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
		std::cerr << color.fail << "ERROR: " << color.none
			<< "Code generator import test \"" << color.name << name << color.none << "\": "
			<< "The generated code differs from the expected code.\n"
			<< "Generated:\n" << color.actual << generatedCode << color.none << '\n'
			<< "Expected:\n" << color.expect << expectedCode << color.none << '\n'
			<< "Input code:\n" << color.code << code << color.none << "\n";

		return false;
	}

	success = coder.GetErrors().size() == expectedErrors;

	if (!success)
	{
		std::cerr << color.fail << "ERROR: " << color.none
			<< "Code generator import test \"" << color.name << name << color.none << "\": "
			<< "The error count differs from the expected count.\n"
			<< "Error messages:\n";

		for (auto& error : coder.GetErrors())
		{
			std::cerr << error << '\n';
		}

		return false;
	}

	return true;
}

bool CoderCppTest::ExportFailure(const char* name, std::string_view code, std::string_view cppCode, int expectedErrors)
{
	StringLexer lexer;
	RangeParser<> parser;
	std::vector<Node> nodes;
	CoderCpp coder;
	std::ostringstream output;
	code >> lexer >> parser >> nodes >> coder >> output;

	bool success = coder.GetExports().size() == 1 || coder.GetExports().empty() && cppCode.empty();

	if (!success)
	{
		std::cerr << color.fail << "ERROR: " << color.none
			<< "Code generator export test \"" << color.name << name << color.none << "\": "
			<< "The code didn't yield a single export.\n";

		return false;
	}

	auto generatedCode = coder.GetExports().front();
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
		std::cerr << color.fail << "ERROR: " << color.none
			<< "Code generator export test \"" << color.name << name << color.none << "\": "
			<< "The generated code differs from the expected code.\n"
			<< "Generated:\n" << color.actual << generatedCode << color.none << '\n'
			<< "Expected:\n" << color.expect << expectedCode << color.none << '\n'
			<< "Input code:\n" << color.code << code << color.none << '\n';

		return false;
	}

	success = coder.GetErrors().size() == expectedErrors;

	if (!success)
	{
		std::cerr << color.fail << "ERROR: " << color.none
			<< "Code generator export test \"" << color.name << name << color.none << "\": "
			<< "The error count differs from the expected count.\n"
			<< "Error messages:\n";

		for (auto& error : coder.GetErrors())
		{
			std::cerr << error << '\n';
		}

		return false;
	}

	return true;
}

using namespace boost::ut;

suite CoderCpp_function_literal_input_type_tests = [] {
	"[1] input"_test = [] {
		expect(s_test.Success("[1] input",
			"[1] f",
			R"cpp(auto f_f(lovela::context& context, l_i8 in);)cpp"
		));
	};

	"[-200] input"_test = [] {
		expect(s_test.Success("[-200] input",
			"[-200] f",
			R"cpp(auto f_f(lovela::context& context, l_i16 in);)cpp"
		));
	};

	"[1.0e30] input"_test = [] {
		expect(s_test.Success("[1.0e30] input",
			"[1.0e30] f",
			R"cpp(auto f_f(lovela::context& context, l_f32 in);)cpp"
		));
	};

	"[1.0e300] input"_test = [] {
		expect(s_test.Success("[1.0e300] input",
			"[1.0e300] f",
			R"cpp(auto f_f(lovela::context& context, l_f64 in);)cpp"
		));
	};
};

suite CoderCpp_function_builtin_input_type_tests = [] {
	"l_i1 input error"_test = [] {
		expect(s_test.Failure("l_i1 input error",
			"[/type/i1] f",
			R"cpp(auto f_f(lovela::context& context, InvalidTypeName in);)cpp",
			1
		));
	};

	"l_i8 input"_test = [] {
		expect(s_test.Success("l_i8 input",
			"[/type/i8] f",
			R"cpp(auto f_f(lovela::context& context, l_i8 in);)cpp"
		));
	};

	"l_i16 input"_test = [] {
		expect(s_test.Success("l_i16 input",
			"[/type/i16] f",
			R"cpp(auto f_f(lovela::context& context, l_i16 in);)cpp"
		));
	};

	"l_i32 input"_test = [] {
		expect(s_test.Success("l_i32 input",
			"[/type/i32] f",
			R"cpp(auto f_f(lovela::context& context, l_i32 in);)cpp"
		));
	};

	"l_i64 input"_test = [] {
		expect(s_test.Success("l_i64 input",
			"[/type/i64] f",
			R"cpp(auto f_f(lovela::context& context, l_i64 in);)cpp"
		));
	};

	"l_i2 input error"_test = [] {
		expect(s_test.Failure("l_i2 input error",
			"[/type/i2] f",
			R"cpp(auto f_f(lovela::context& context, InvalidTypeName in);)cpp",
			1
		));
	};

	"l_u1 input error"_test = [] {
		expect(s_test.Failure("l_u1 input error",
			"[/type/u1] f",
			R"cpp(auto f_f(lovela::context& context, InvalidTypeName in);)cpp",
			1
		));
	};

	"l_u8 input"_test = [] {
		expect(s_test.Success("l_u8 input",
			"[/type/u8] f",
			R"cpp(auto f_f(lovela::context& context, l_u8 in);)cpp"
		));
	};

	"l_u16 input"_test = [] {
		expect(s_test.Success("l_u16 input",
			"[/type/u16] f",
			R"cpp(auto f_f(lovela::context& context, l_u16 in);)cpp"
		));
	};

	"l_u32 input"_test = [] {
		expect(s_test.Success("l_u32 input",
			"[/type/u32] f",
			R"cpp(auto f_f(lovela::context& context, l_u32 in);)cpp"
		));
	};

	"l_u64 input"_test = [] {
		expect(s_test.Success("l_u64 input",
			"[/type/u64] f",
			R"cpp(auto f_f(lovela::context& context, l_u64 in);)cpp"
		));
	};

	"l_f16 input error"_test = [] {
		expect(s_test.Failure("l_f16 input error",
			"[/type/f16] f",
			R"cpp(auto f_f(lovela::context& context, InvalidTypeName in);)cpp",
			1
		));
	};

	"l_f32 input"_test = [] {
		expect(s_test.Success("l_f32 input",
			"[/type/f32] f",
			R"cpp(auto f_f(lovela::context& context, l_f32 in);)cpp"
		));
	};

	"l_f64 input"_test = [] {
		expect(s_test.Success("l_f64 input",
			"[/type/f64] f",
			R"cpp(auto f_f(lovela::context& context, l_f64 in);)cpp"
		));
	};
};

suite CoderCpp_function_tagged_input_type_tests = [] {
	"[#1] input"_test = [] {
		expect(s_test.Success("[#1] input",
			"[#1] f",
			R"cpp(
template <typename Tag1>
auto f_f(lovela::context& context, Tag1 in);)cpp"
		));
	};
};

suite CoderCpp_function_literal_output_type_tests = [] {
	"[1] output"_test = [] {
		expect(s_test.Success("[1] output",
			"f [1]",
			R"cpp(l_i8 f_f(lovela::context& context, auto in);)cpp"
		));
	};
};

suite CoderCpp_function_builtin_output_type_tests = [] {
	"l_i1 output error"_test = [] {
		expect(s_test.Failure("l_i1 output error",
			"f [/type/i1]",
			R"cpp(InvalidTypeName f_f(lovela::context& context, auto in);)cpp",
			1
		));
	};

	"l_i8 output"_test = [] {
		expect(s_test.Success("l_i8 output",
			"f [/type/i8]",
			R"cpp(l_i8 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_i16 output"_test = [] {
		expect(s_test.Success("l_i16 output",
			"f [/type/i16]",
			R"cpp(l_i16 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_i32 output"_test = [] {
		expect(s_test.Success("l_i32 output",
			"f [/type/i32]",
			R"cpp(l_i32 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_i64 output"_test = [] {
		expect(s_test.Success("l_i64 output",
			"f [/type/i64]",
			R"cpp(l_i64 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_i2 output error"_test = [] {
		expect(s_test.Failure("l_i2 output error",
			"f [/type/i2]",
			R"cpp(InvalidTypeName f_f(lovela::context& context, auto in);)cpp",
			1
		));
	};

	"l_u1 output error"_test = [] {
		expect(s_test.Failure("l_u1 output error",
			"f [/type/u1]",
			R"cpp(InvalidTypeName f_f(lovela::context& context, auto in);)cpp",
			1
		));
	};

	"l_u8 output"_test = [] {
		expect(s_test.Success("l_u8 output",
			"f [/type/u8]",
			R"cpp(l_u8 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_u16 output"_test = [] {
		expect(s_test.Success("l_u16 output",
			"f [/type/u16]",
			R"cpp(l_u16 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_u32 output"_test = [] {
		expect(s_test.Success("l_u32 output",
			"f [/type/u32]",
			R"cpp(l_u32 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_u64 output"_test = [] {
		expect(s_test.Success("l_u64 output",
			"f [/type/u64]",
			R"cpp(l_u64 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_f16 output error"_test = [] {
		expect(s_test.Failure("l_f16 output error",
			"f [/type/f16]",
			R"cpp(InvalidTypeName f_f(lovela::context& context, auto in);)cpp",
			1
		));
	};

	"l_f32 output"_test = [] {
		expect(s_test.Success("l_f32 output",
			"f [/type/f32]",
			R"cpp(l_f32 f_f(lovela::context& context, auto in);)cpp"
		));
	};

	"l_f64 output"_test = [] {
		expect(s_test.Success("l_f64 output",
			"f [/type/f64]",
			R"cpp(l_f64 f_f(lovela::context& context, auto in);)cpp"
		));
	};
};

suite CoderCpp_function_tagged_output_type_tests = [] {
	"[#1] output"_test = [] {
		expect(s_test.Success("[#1] output",
			"f [#1]",
			R"cpp(template <typename Tag1> Tag1 f_f(lovela::context& context, auto in);)cpp"
		));
	};
};

suite CoderCpp_function_literal_param_type_tests = [] {
	"[1] param"_test = [] {
		expect(s_test.Success("[1] param",
			"f ([1])",
			R"cpp(auto f_f(lovela::context& context, auto in, l_i8 param1);)cpp"
		));
	};
};

suite CoderCpp_function_builtin_param_type_tests = [] {
	"l_i1 param error"_test = [] {
		expect(s_test.Failure("l_i1 param error",
			"f ([/type/i1])",
			R"cpp(auto f_f(lovela::context& context, auto in, InvalidTypeName param1);)cpp",
			1
		));
	};

	"l_i32 param"_test = [] {
		expect(s_test.Success("l_i32 param",
			"f ([/type/i32])",
			R"cpp(auto f_f(lovela::context& context, auto in, l_i32 param1);)cpp"
		));
	};

	"l_i2 param error"_test = [] {
		expect(s_test.Failure("l_i2 param error",
			"f ([/type/i2])",
			R"cpp(auto f_f(lovela::context& context, auto in, InvalidTypeName param1);)cpp",
			1
		));
	};

	"l_u8 param"_test = [] {
		expect(s_test.Success("l_u8 param",
			"f ([/type/u8])",
			R"cpp(auto f_f(lovela::context& context, auto in, l_u8 param1);)cpp"
		));
	};

	"l_u64 param"_test = [] {
		expect(s_test.Success("l_u64 param",
			"f ([/type/u64])",
			R"cpp(auto f_f(lovela::context& context, auto in, l_u64 param1);)cpp"
		));
	};

	"l_f16 param error"_test = [] {
		expect(s_test.Failure("l_f16 param error",
			"f ([/type/f16])",
			R"cpp(auto f_f(lovela::context& context, auto in, InvalidTypeName param1);)cpp",
			1
		));
	};

	"l_f32 param"_test = [] {
		expect(s_test.Success("l_f32 param",
			"f ([/type/f32])",
			R"cpp(auto f_f(lovela::context& context, auto in, l_f32 param1);)cpp"
		));
	};
};

suite CoderCpp_function_tagged_param_type_tests = [] {
	"[#1] param"_test = [] {
		expect(s_test.Success("[#1] param",
			"f ([#1])",
			R"cpp(template <typename Tag1> auto f_f(lovela::context& context, auto in, Tag1 param1);)cpp"
		));
	};

	"[#1] [#2] param"_test = [] {
		expect(s_test.Success("[#1] [#2] param",
			"f ([#1], [#2])",
			R"cpp(template <typename Tag1, typename Tag2> auto f_f(lovela::context& context, auto in, Tag1 param1, Tag2 param2);)cpp"
		));
	};

	"[#1] [#second] param"_test = [] {
		expect(s_test.Success("[#1] [#second] param",
			"f ([#1], [#second])",
			R"cpp(template <typename Tag1, typename Tagsecond> auto f_f(lovela::context& context, auto in, Tag1 param1, Tagsecond param2);)cpp"
		));
	};

	"[#first] [#second] param"_test = [] {
		expect(s_test.Success("[#first] [#second] param",
			"f ([#first], [#second])",
			R"cpp(template <typename Tagfirst, typename Tagsecond> auto f_f(lovela::context& context, auto in, Tagfirst param1, Tagsecond param2);)cpp"
		));
	};
};

suite CoderCpp_function_declaration_tests = [] {
	"trivial function"_test = [] { 
		expect(s_test.Success("trivial function", 
			"func",
			R"cpp(auto f_func(lovela::context& context, auto in);)cpp"
		));
	};

	"function with return type"_test = [] { 
		expect(s_test.Success("function with return type", 
			"func [type]",
			R"cpp(t_type f_func(lovela::context& context, auto in);)cpp"
		));
	};

	"function with object type"_test = [] { 
		expect(s_test.Success("function with object type", 
			"[type] func",
			R"cpp(auto f_func(lovela::context& context, t_type in);)cpp"
		));
	};

	"function with untyped parameter"_test = [] { 
		expect(s_test.Success("function with untyped parameter", 
			"func (arg)",
			R"cpp(auto f_func(lovela::context& context, auto in, auto p_arg);)cpp"
		));
	};

	"function with typed parameter"_test = [] { 
		expect(s_test.Success("function with typed parameter", 
			"func (arg [type])",
			R"cpp(auto f_func(lovela::context& context, auto in, t_type p_arg);)cpp"
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

suite CoderCpp_function_call_tests = [] {
	"function call"_test = [] { 
		expect(s_test.Success("function call", 
			"[/type/i8] func [/type/i8]: f(1, 'a', g).", 
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

suite CoderCpp_exported_functions_with_implementation_tests = [] {
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

suite CoderCpp_exported_functions_tests = [] {
	"exported function C"_test = [] { 
		expect(s_test.ExportSuccess("exported function C", 
			"<- 'C' [/type/i32] ex [/type/i32]", 
			"LOVELA_API_C l_i32 ex(l_i32 in)"
		));
	};

	"exported function C++"_test = [] { 
		expect(s_test.ExportSuccess("exported function C++", 
			"<- 'C++' [/type/i32] ex [/type/i32]", 
			"LOVELA_API_CPP l_i32 ex(l_i32 in)"
		));
	};

	"exported function C Dynamic"_test = [] { 
		expect(s_test.ExportSuccess("exported function C Dynamic", 
			"<- 'C Dynamic' [/type/i32] ex [/type/i32]", 
			"LOVELA_API_C LOVELA_API_DYNAMIC_EXPORT l_i32 ex(l_i32 in)"
		));
	};
};

suite CoderCpp_imported_functions_tests = [] {
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
			"-> 'C' [/type/i8] im [/type/i8]",
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
			"-> 'C Dynamic' [/type/i8] im [/type/i8]",
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

suite CoderCpp_imported_standard_functions_tests = [] {
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
			"-> 'Standard C++' std/puts", 
			"cstdio"
		));
	};

	"imported function Standard C++ cstlib"_test = [] { 
		expect(s_test.ImportSuccess("imported function Standard C++ cstlib", 
			"-> 'Standard C++' std/atof", 
			"cstdlib"
		));
	};

	"imported function Standard C++ cstring"_test = [] { 
		expect(s_test.ImportSuccess("imported function Standard C++ cstring", 
			"-> 'Standard C++' std/strcpy", 
			"cstring"
		));
	};

	"imported function Standard C++ cmath"_test = [] { 
		expect(s_test.ImportSuccess("imported function Standard C++ cmath", 
			"-> 'Standard C++' std/sin", 
			"cmath"
		));
	};
};

suite CoderCpp_main_function_tests = [] {
	"main and export"_test = [] { 
		expect(s_test.Success("main and export", 
			"<- [/type/i32] ex [/type/i32]: + 1. : 1 ex.", 
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
			"-> [/type/i8]# puts [/type/i32]. : 'Hello, Wordl!' puts.", 
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
