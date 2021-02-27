#pragma once
#include <string>
#include <vector>
#include <istream>
#include "Token.h"

class ILexer
{
public:
	struct Error
	{
		enum class Code
		{
			UnknownError,
			SyntaxError,
			OpenComment,
			OpenStringLiteral,
			StringFieldIllformed,
			StringFieldUnknown,
		} code{};
		int line{};
		int column{};
		std::wstring message;
	};

	virtual std::vector<Token> Lex(std::wistream& charStream) noexcept = 0;
	virtual const std::vector<Error>& GetErrors() noexcept = 0;
};
