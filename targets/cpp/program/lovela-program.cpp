#include "lovela-program.h"

LOVELA_IMPORT int32_t puts(int8_t* in);

int32_t f_puts(lovela::context& context, int8_t* in)
{
	context;
	return puts(in);
}

lovela::None lovela::main(lovela::context& context, lovela::None in)
{
	context;
	auto& v1 = in; v1;
	const auto v2 = f_puts(context, "Hello, Wordl!"); v2;
	return {};
}

