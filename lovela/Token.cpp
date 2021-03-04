#include "pch.h"
#include "Token.h"

bool Token::operator==(const Token& rhs) const noexcept
{
	// Compare everything but the code location
	return rhs.type == type && rhs.value == value;
}
