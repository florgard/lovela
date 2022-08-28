#pragma once
#include "Token.h"

class ITokenIterator
{
public:
	virtual ~ITokenIterator() = default;
	virtual [[nodiscard]] const Token& operator*() const noexcept = 0;
	virtual [[nodiscard]] const Token* operator->() const noexcept = 0;
	virtual ITokenIterator& operator++() = 0;
	virtual ITokenIterator& operator++(int) = 0;
	virtual [[nodiscard]] bool empty() const noexcept = 0;
};
