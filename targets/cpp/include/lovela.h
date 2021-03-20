#pragma once
#include <string>
#include <variant>

struct lovela_error
{
	int code{};
	std::wstring message;
};
