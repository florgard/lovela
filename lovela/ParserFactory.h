#pragma once
#include "ILexer.h"
#include "IParser.h"

class ParserFactory
{
public:
	static std::unique_ptr<IParser> Create(TokenGenerator&& tokenGenerator, std::string_view id = {}) noexcept;
};
