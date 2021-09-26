export module LexerFactory;

export import ILexer;
export import Lexer;
import <string_view>;
import <memory>;
import <iostream>;

export class LexerFactory
{
public:
	static std::unique_ptr<ILexer> Create(std::wistream& charStream, std::string_view id = {}) noexcept
	{
		static_cast<void>(id);
		return std::unique_ptr<Lexer>(new Lexer(charStream));
	}
};
