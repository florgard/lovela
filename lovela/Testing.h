#pragma once
#include "ILexer.h"

void Test(const char* name, ILexer& lexer, std::wstring_view code, const std::vector<Token>& expectedTokens, const std::vector<ILexer::Error>& expectedErrors);
