#include "libpu8.h"
#include "../../lovela/lovela-dependencies.h"
#include "../../lovela/LexerFactory.h"
#include "../../lovela/Parser.h"
#include "../../lovela/CodeGeneratorFactory.h"

int main_utf8(int argc, char** argv)
{
	auto lexer = LexerFactory::Create(std::cin);
	RangeParser parser;
	parser.Initialize(lexer->Lex());
	auto nodes = to_vector(parser.Parse());
	auto codeGen = CodeGeneratorFactory::Create(std::cout, "Cpp");

	for (auto&& node : parser.Parse())
	{
		Traverse<Node>::DepthFirstPostorder(node, [&](auto& node) { codeGen->Visit(node); });
	}

	return 0;
}
