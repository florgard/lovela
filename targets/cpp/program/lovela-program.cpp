
#include "lovela-program.h"

auto f_puts(lovela::context& context, auto in)
{
  static_cast<void>(context);
  return puts(in);
}

lovela::None lovela::main(lovela::context& context, lovela::None in)
{
  static_cast<void>(context);
  auto& v1 = in; static_cast<void>(v1);
  const auto v2 = f_puts(context, "Hello, World!"); static_cast<void>(v2);
  return {};
}
