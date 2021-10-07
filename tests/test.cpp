import boost.ut;
import Lexer.Token;
import ILexer;
import LexerBase;
import LexerFactory;
import Parser.Node;
import IParser;
import ParserFactory;
import CodeGeneratorFactory;
import Utility;
import Utility.StaticMap;

using namespace boost::ut;

int main()
{
	expect(true);

	//Token::Type a = LexerBase::GetTokenType('(');
	auto b = Token::Type::ParenRoundOpen;
	//EXPECT_EQ(a, b);
}
