#include "Parser.h"
#include <iostream>

Parser::Parser(TokenGenerator&& tokenGenerator) noexcept : tokenGenerator(std::move(tokenGenerator))
{
}

Node Parser::Parse() noexcept
{
	for (const auto& token : tokenGenerator)
	{
		std::cout << "Consuming token " << static_cast<int>(token.type) << '\n';
	}

	return {};
}
