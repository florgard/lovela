#pragma once
#include "Node.h"

struct IParser : public IEnumerator<Token>
{
	using Generator = tl::generator<Node>;

	[[nodiscard]] virtual Generator Parse() noexcept = 0;
};
