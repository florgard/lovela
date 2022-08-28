#pragma once
#include "ICodeGenerator.h"

class CodeGeneratorFactory
{
public:
	static std::unique_ptr<ICodeGenerator> Create(std::wostream& stream, std::string_view id) noexcept;
};
