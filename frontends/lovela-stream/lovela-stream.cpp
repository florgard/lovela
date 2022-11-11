#include <cstdint>
#include <exception>
#include <source_location>
#include <iso646.h>
#include <utility>
#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <set>
#include <map>
#include <deque>
#include <algorithm>
#include <type_traits>
#include <compare>
#include <memory>
#include <functional>
#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>
#include <cwctype>
#include <cassert>
#include <stdexcept>
#include "../../lovela/utfcpp/utf8.h"
#include "../../lovela/generator.hpp"
#include "../../lovela/Utility.h"
#include "../../lovela/Algorithm.h"
#include "../../lovela/LexerFactory.h"
#include "../../lovela/ParserFactory.h"
#include "../../lovela/CodeGeneratorFactory.h"
#include "libpu8.h"

int main_utf8(int argc, char** argv)
{
	auto lexer = LexerFactory::Create(std::cin);
	auto parser = ParserFactory::Create(lexer->Lex());
	auto codeGen = CodeGeneratorFactory::Create(std::cout, "Cpp");

	for (auto&& node : parser->Parse())
	{
		Traverse<Node>::DepthFirstPostorder(node, [&](auto& node) { codeGen->Visit(node); });
	}

	return 0;
}
