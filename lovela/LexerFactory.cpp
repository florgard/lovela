#include "pch.h"
#include "LexerFactory.h"
#include "Lexer.h"

std::unique_ptr<ILexer> LexerFactory::Create(std::wistream& charStream, std::string_view id) noexcept
{
	static_cast<void>(id);
	return std::unique_ptr<Lexer>(new Lexer(charStream));
}
