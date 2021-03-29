#include "lovela-program.h"

template <typename In>
auto f_puts(lovela::context& context, In in)
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

