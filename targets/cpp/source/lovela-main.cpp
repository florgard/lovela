#include "lovela.h"

#ifdef _WIN32

int wmain(int argc, wchar_t* argv[])
{
	lovela::context context;
	for (int i = 1; i < argc; ++i)
	{
		context.parameters.emplace_back(lovela::stream::to_string(argv[i]));
	}
	lovela::None in;
	lovela::main(context, in);
	return context.error.code;
}

#else

int main(int argc, char* argv[])
{
	lovela::context context{ .parameters{argv + 1, argv + argc} };
	lovela::None in;
	lovela::main(context, in);
	return context.error.code;
}

#endif
