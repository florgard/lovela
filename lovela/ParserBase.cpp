#include "pch.h"
#include "ParserBase.h"

ParserBase::ParserBase(ITokenGenerator&& tokenGenerator) noexcept
	: _tokenGenerator(std::move(tokenGenerator))
	, _tokenIterator(_tokenGenerator.begin())
{
}
