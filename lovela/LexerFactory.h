#pragma once
#include "ILexer.h"

class LexerFactory
{
public:
	static std::unique_ptr<ILexer> Create(std::istream& charStream, std::string_view id = {}) noexcept;
};
