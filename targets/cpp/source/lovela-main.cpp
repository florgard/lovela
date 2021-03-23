#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include "lovela.h"
#include <codecvt>

#ifdef _WIN32

std::string to_utf8(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
	return convert.to_bytes(str);
}

int wmain(int argc, wchar_t* argv[])
{
	lovela::context context;
	for (int i = 1; i < argc; i++)
	{
		context.parameters.emplace_back(to_utf8(argv[i]));
	}
	lovela::main(context);
	return context.error.code;
}

#else

int main(int argc, char* argv[])
{
	lovela::context context{ .parameters{argv + 1, argv + argc} };
	lovela::main(context);
	return context.error.code;
}

#endif
