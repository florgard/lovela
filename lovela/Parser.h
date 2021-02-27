#pragma once
#include <vector>
#include "Token.h"

class Parser
{
public:
	void Parse(const std::vector<Token>& tokens) noexcept;
};
