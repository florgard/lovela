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
	auto& v1 = in; static_cast<void>(v1);
	const auto v2 = f_puts(context, "Hello, Wordl!"); static_cast<void>(v2);
	return {};
}

