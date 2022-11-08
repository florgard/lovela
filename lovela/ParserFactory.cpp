#include "pch.h"
#include "ParserFactory.h"
#include "Parser.h"
#include "TokenIterator.h"

std::unique_ptr<IParser> ParserFactory::Create(ITokenGenerator&& tokenGenerator, std::string_view id) noexcept
{
	static_cast<void>(id);
	return std::unique_ptr<Parser>(new Parser(std::move(tokenGenerator)));
}
