export module LexerFactory;

export import ILexer;
export import Lexer;
import <iostream>;
import <memory>;

export class LexerFactory
{
public:
	static std::unique_ptr<ILexer> Create(std::wistream& charStream, const char* id) noexcept
	{
		static_cast<void>(id);
		return std::unique_ptr<Lexer>(new Lexer(charStream));
	}
};
