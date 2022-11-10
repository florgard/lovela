#pragma once
#include "ICodeGenerator.h"

class CodeGeneratorFactory
{
public:
	static std::unique_ptr<ICodeGenerator> Create(std::ostream& stream, std::string_view id) noexcept;
};
