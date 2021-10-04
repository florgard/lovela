export module ICodeGenerator;

export import Parser.Node;
import <string>;
import <vector>;
import <iostream>;

export class ICodeGenerator
{
public:
	virtual ~ICodeGenerator() = default;

	virtual void Visit(Node & node) = 0;

	virtual [[nodiscard]] const std::vector<std::wstring>& GetErrors() const noexcept = 0;
	virtual [[nodiscard]] const std::vector<std::wstring>& GetImports() const noexcept = 0;
	virtual [[nodiscard]] const std::vector<std::wstring>& GetExports() const noexcept = 0;

	virtual void GenerateImportsFile(std::wostream& file) const noexcept = 0;
	virtual void GenerateExportsFile(std::wostream& file) const noexcept = 0;
};
