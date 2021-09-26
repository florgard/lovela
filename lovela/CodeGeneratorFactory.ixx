export module CodeGeneratorFactory;

export import ICodeGenerator;
export import CodeGenerator.Cpp;
import <string_view>;
import <memory>;

export class CodeGeneratorFactory
{
public:
	static std::unique_ptr<ICodeGenerator> Create(std::wostream& stream, std::string_view id) noexcept
	{
		if (id == "Cpp")
		{
			return std::unique_ptr<CodeGeneratorCpp>(new CodeGeneratorCpp(stream));
		}

		return {};
	}
};
