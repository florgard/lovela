#include "pch.h"
#include "ParserBase.h"

ParserBase::ParserBase(std::unique_ptr<ITokenIterator> tokenIterator) noexcept : tokenIterator(std::move(tokenIterator))
{
}
