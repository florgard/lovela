#pragma once
#include "ILexer.h"

class Node
{

};

class Parser
{
	TokenGenerator tokenGenerator;

public:
	Parser(TokenGenerator&& tokenGenerator) noexcept;

	Node Parse() noexcept;
};
