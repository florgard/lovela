#pragma once
#include <vector>
#include <istream>
#include "Token.h"

std::vector<Token> Lex(std::istream& charStream) noexcept;
