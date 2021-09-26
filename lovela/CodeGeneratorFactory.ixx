export module CodeGeneratorFactory;

export import ICodeGenerator;
export import CodeGenerator.Cpp;
import <memory>;

export class CodeGeneratorFactory
{
public:
	static std::unique_ptr<ICodeGenerator> Create(std::wostream& stream, const char* id = nullptr) noexcept
	{
		static_cast<void>(id);
		return std::unique_ptr<CodeGeneratorCpp>(new CodeGeneratorCpp(stream));
	}
};
