export module CodeGenerator.Cpp:StandardCDeclarations;

import <string>;
import <string_view>;
import <vector>;

export class StandardCDeclarations
{
public:
	static void GetHeader(std::vector<std::wstring>& headers, std::wstring_view function);
};
