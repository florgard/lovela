export module CodeGenerator.Cpp:StandardCppDeclarations;

import <string>;
import <string_view>;
import <iostream>;
import <sstream>;
import <vector>;
import <array>;
import <set>;
import <map>;
import <functional>;
import <algorithm>;

export class StandardCppDeclarations
{
public:
	static void GetHeader(std::vector<std::wstring>& headers, std::wstring_view function);
};
