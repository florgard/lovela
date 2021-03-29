#include "pch.h"
#include "StandardCppDeclarations.h"

void StandardCppDeclarations::GetHeader(std::vector<std::wstring>& headers, std::wstring_view function)
{
	// TODO
	static const std::map<std::string, std::string> declarations{
	};

	auto iter = declarations.find(to_string(function));
	if (iter != declarations.end())
	{
		headers.emplace_back(to_wstring(iter->second));
	}
}
