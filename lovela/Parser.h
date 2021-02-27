#pragma once
#include "ILexer.h"

class Parser
{
public:
	void Parse(TokenGenerator tokenGenerator) noexcept;
};
