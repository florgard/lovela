#include "lovela-program.h"

LOVELA_IMPORT l_i32 puts(l_cstr in);

l_i32 f_puts(lovela::context& context, l_cstr in)
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

