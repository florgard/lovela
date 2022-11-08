#pragma once
#include "IParser.h"
#include "TokenGenerator.h"

class ParserFactory
{
public:
	static std::unique_ptr<IParser> Create(TokenGenerator&& tokenGenerator, std::string_view id = {}) noexcept;
};
