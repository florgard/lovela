export module CodeGenerator.Cpp:StandardCDeclarations;

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

export class StandardCDeclarations
{
public:
	static void GetHeader(std::vector<std::wstring>& headers, std::wstring_view function);
};
