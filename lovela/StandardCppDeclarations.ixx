export module CodeGenerator.Cpp:StandardCppDeclarations;

import <string>;
import <string_view>;
import <vector>;

export class StandardCppDeclarations
{
public:
	static void GetHeader(std::vector<std::wstring>& headers, std::wstring_view function);
};
