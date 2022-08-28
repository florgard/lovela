#include "pch.h"
#include "CodeGeneratorFactory.h"
#include "CodeGeneratorCpp.h"

std::unique_ptr<ICodeGenerator> CodeGeneratorFactory::Create(std::wostream& stream, std::string_view id) noexcept
{
	if (id == "Cpp")
	{
		return std::unique_ptr<CodeGeneratorCpp>(new CodeGeneratorCpp(stream));
	}

	return {};
}
