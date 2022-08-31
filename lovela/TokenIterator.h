#pragma once
#include "ITokenIterator.h"
#include "ITokenGenerator.h"

template <class TokenGenerator = ITokenGenerator, typename Iterator = TokenGenerator::iterator>
class TokenIterator : public ITokenIterator
{
	TokenGenerator gen;
	Iterator iter;

public:
	TokenIterator(TokenGenerator&& tokenGenerator) noexcept : gen(std::move(tokenGenerator)), iter(gen.begin())
	{
	}

	[[nodiscard]] const Token& operator*() const noexcept override
	{
		return iter.operator*();
	}

	[[nodiscard]] const Token* operator->() const noexcept override
	{
		return iter.operator->();
	}

	ITokenIterator& operator++() override
	{
		iter.operator++();
		return *this;
	}

	ITokenIterator& operator++(int) override
	{
		iter.operator++(0);
		return *this;
	}

	[[nodiscard]] bool empty() const noexcept override
	{
		return iter == Iterator{};
	}
};
