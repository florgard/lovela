export module ParserFactory;

export import IParser;
export import Parser;
import ILexer;
import <memory>;

export class ParserFactory
{
public:
	static std::unique_ptr<IParser> Create(TokenGenerator&& tokenGenerator, const char* id = nullptr) noexcept
	{
		static_cast<void>(id);
		return std::unique_ptr<Parser>(new Parser(std::move(tokenGenerator)));
	}
};
