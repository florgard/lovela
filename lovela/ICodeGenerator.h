#pragma once
#include "Parser.h"

class ICodeGenerator
{
public:
	virtual ~ICodeGenerator() = default;

	virtual void Visit(Node& node) = 0;

	virtual [[nodiscard]] const std::vector<std::string>& GetErrors() const noexcept = 0;
	virtual [[nodiscard]] const std::vector<std::string>& GetImports() const noexcept = 0;
	virtual [[nodiscard]] const std::vector<std::string>& GetExports() const noexcept = 0;

	virtual void GenerateProgramFile(std::ostream& file) const = 0;
	virtual void GenerateImportsFile(std::ostream& file) const = 0;
	virtual void GenerateExportsFile(std::ostream& file) const = 0;
};

inline void operator>>(std::ranges::range auto& input, ICodeGenerator& codeGen)
{
	Traverse<Node>::DepthFirstPostorder(input, [&](Node& node) { codeGen.Visit(node); });
}

inline void operator>>(Parser::Generator input, ICodeGenerator& codeGen)
{
	Traverse<Node>::DepthFirstPostorder(input, [&](Node& node) { codeGen.Visit(node); });
}
