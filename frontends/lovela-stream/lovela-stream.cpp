#include "libpu8.h"
#include "../../lovela/lovela-dependencies.h"
#include "../../lovela/Lexer.h"
#include "../../lovela/Parser.h"
#include "../../lovela/CodeGeneratorFactory.h"

int main_utf8(int argc, char** argv)
{
	std::unique_ptr<StreamLexer> lexer;
	RangeParser parser;
	std::vector<Node> nodes;
	std::cin >> lexer >> parser >> nodes;
	auto codeGen = CodeGeneratorFactory::Create(std::cout, "Cpp");

	for (auto&& node : parser.Parse())
	{
		Traverse<Node>::DepthFirstPostorder(node, [&](auto& node) { codeGen->Visit(node); });
	}

	return 0;
}
