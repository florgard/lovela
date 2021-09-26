export module ParserBase;

import Token;
import ILexer;
import IParser;
import <vector>;
import <set>;

export
{
	class ParserBase : public IParser
	{
	public:
		ParserBase(TokenGenerator&& tokenGenerator) noexcept;

		[[nodiscard]] const std::vector<Error>& GetErrors() noexcept override
		{
			return errors;
		}

		void Assert();
		void Assert(Token::Type type);
		void Assert(const std::set<Token::Type>& types);
		void Skip();
		void Skip(Token::Type type);
		void Skip(const std::set<Token::Type>& types);
		[[nodiscard]] bool IsToken(Token::Type type);
		[[nodiscard]] bool IsToken(const std::set<Token::Type>& types);

		void Expect(Token::Type type);
		void Expect(const std::set<Token::Type>& types);
		[[nodiscard]] bool Accept(Token::Type type);
		[[nodiscard]] bool Accept(const std::set<Token::Type>& types);
		[[nodiscard]] bool Peek();
		[[nodiscard]] bool Peek(Token::Type type);
		[[nodiscard]] bool Peek(const std::set<Token::Type>& types);

	protected:
		TokenGenerator tokenGenerator;
		//TokenIterator tokenIterator;
		Token currentToken;
		std::vector<Error> errors;

	public:
		struct TokenIterator
		{
			virtual ~TokenIterator() = default;
			virtual [[nodiscard]] const Token& operator*() const noexcept = 0;
			virtual [[nodiscard]] const Token* operator->() const noexcept = 0;
			virtual TokenIterator& operator++() = 0;
			virtual TokenIterator& operator++(int) = 0;
			virtual [[nodiscard]] bool empty() const noexcept = 0;
		};

		TokenIterator& GetTokenIterator() noexcept { return *tokenIteratorPtr; };
		const TokenIterator& GetTokenIterator() const noexcept { return *tokenIteratorPtr; };
		void SetTokenIterator(std::unique_ptr<TokenIterator> ptr) noexcept { tokenIteratorPtr = std::move(ptr); };

	private:
		std::unique_ptr<TokenIterator> tokenIteratorPtr;
	};
}
