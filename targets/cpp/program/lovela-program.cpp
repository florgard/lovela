#include "lovela-program.h"

int32_t f_ex(lovela::context& context, int32_t in)
{
	context;
	auto& v1 = in; v1;
	const auto v2 = v1 + 1 ;
	return v2;
}

int32_t ex(int32_t in)
{
	lovela::context context;
	return f_ex(context, in);
}

lovela::None lovela::main(lovela::context& context, lovela::None in)
{
	context;
	auto& v1 = in; v1;
	const auto v2 = f_ex( context, 1 ) ;
	return {};
}

