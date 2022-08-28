#pragma once
#include "IParser.h"
#include "ITokenGenerator.h"

class ParserFactory
{
public:
	static std::unique_ptr<IParser> Create(ITokenGenerator&& tokenGenerator, std::string_view id = {}) noexcept;
};
