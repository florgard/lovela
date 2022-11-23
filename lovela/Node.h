#pragma once
#include "Token.h"

struct NameSpace
{
	std::vector<std::string> parts;
	bool root{};

	[[nodiscard]] constexpr auto operator<=>(const NameSpace& rhs) const noexcept = default;

	[[nodiscard]] void Print(std::ostream& stream) const
	{
		if (root)
		{
			stream << Token::Constant::NameSpaceSeparator;
		}

		for (auto& part : parts)
		{
			stream << part << Token::Constant::NameSpaceSeparator;
		}
	}
};

struct TypeSpec
{
	enum class Kind
	{
		Any,
		None,
		Tagged,
		Named,
		Primitive,
		Invalid,
	} kind{};

	std::string name;
	NameSpace nameSpace{};
	std::vector<size_t> arrayDims;

	struct Primitive
	{
		unsigned char bits{};
		bool signedType{};
		bool floatType{};

		[[nodiscard]] constexpr auto operator<=>(const Primitive& rhs) const noexcept = default;

		[[nodiscard]] void Print(std::ostream& stream) const
		{
			stream << '[' << static_cast<unsigned int>(bits) << ',' << signedType << ',' << floatType << ']';
		}
	} primitive{};

	[[nodiscard]] constexpr bool Is(Kind k) const noexcept { return kind == k; }

	[[nodiscard]] constexpr auto operator<=>(const TypeSpec& rhs) const noexcept = default;

	[[nodiscard]] void PrintPrimitiveName(std::ostream& stream) const
	{
		stream << (primitive.floatType ? 'f' : (primitive.signedType ? 'i' : 'u')) << static_cast<unsigned int>(primitive.bits);
	}

	[[nodiscard]] std::string GetQualifiedName() const
	{
		std::ostringstream s;

		s << '[';

		nameSpace.Print(s);

		if (Is(Kind::Primitive))
		{
			PrintPrimitiveName(s);
		}
		else
		{
			s << name;
		}

		s << ']';

		for (auto& length : arrayDims)
		{
			s << '#';

			if (length)
			{
				s << length;
			}
		}

		return s.str();
	}

	[[nodiscard]] void Print(std::ostream& stream) const
	{
		stream << '[' << to_string(kind) << ',' << name << ',';
		nameSpace.Print(stream);
		stream << ',' << '[';
		for (auto& length : arrayDims)
		{
			stream << length << ',';
		}
		stream << ']' << ',';
		primitive.Print(stream);
		stream << ']';
	}

private:
	static constexpr const char* noneTypeName = "()";
};

struct VariableDeclaration
{
	std::string name;
	TypeSpec type{};

	[[nodiscard]] auto operator<=>(const VariableDeclaration& rhs) const noexcept = default;

	[[nodiscard]] void Print(std::ostream& stream) const
	{
		stream << '[' << name << ',';
		type.Print(stream);
		stream << ']';
	}
};

using ParameterList = std::vector<std::shared_ptr<VariableDeclaration>>;

struct ApiSpec
{
	static constexpr int None = 0;
	static constexpr int Import = 1 << 0;
	static constexpr int Export = 1 << 1;
	static constexpr int Dynamic = 1 << 2;
	static constexpr int Standard = 1 << 3;
	static constexpr int C = 1 << 4;
	static constexpr int Cpp = 1 << 5;

	constexpr ApiSpec() noexcept = default;
	constexpr ApiSpec(int flags) noexcept : flags(flags) {}

	[[nodiscard]] constexpr auto operator<=>(const ApiSpec& rhs) const noexcept = default;
	
	constexpr void Set(int flag)
	{
		flags |= flag;
	}

	constexpr bool Is(int flag) const
	{
		return (flags & flag) == flag;
	}

	constexpr bool IsExplicit() const
	{
		return flags > Export;
	}

	[[nodiscard]] void Print(std::ostream& stream) const
	{
		stream << '[' << flags << ']';
	}

private:
	int flags{};
};

struct FunctionDeclaration
{
	std::string name;
	NameSpace nameSpace{};
	TypeSpec outType{};
	TypeSpec inType{};
	ParameterList parameters{};
	ApiSpec api{};

	[[nodiscard]] void Print(std::ostream& stream) const
	{
		stream << '[' << name << ',';
		nameSpace.Print(stream);
		stream << ',';
		outType.Print(stream);
		stream << ',';
		inType.Print(stream);
		stream << ',' << '[';
		for (auto& param : parameters)
		{
			param->Print(stream);
			stream << ',';
		}
		stream << ']' << ',';
		api.Print(stream);
		stream << ']';
	}
};

struct Node
{
	enum class Type
	{
		Empty,
		Error,
		FunctionDeclaration,
		Expression,
		ExpressionInput,
		Tuple,
		Literal,
		FunctionCall,
		VariableReference,
		BinaryOperation,
	} type{};

	std::string value;
	TypeSpec outType{};
	Token token{};

	// Function declaration
	NameSpace nameSpace{};
	TypeSpec inType{};
	ParameterList parameters{};
	ApiSpec api{};

	// Function call
	std::shared_ptr<FunctionDeclaration> callee;

	std::unique_ptr<Node> left;
	std::unique_ptr<Node> right;

	struct Error
	{
		enum class Code
		{
			NoError,
			ParseError,
		} code{};

		std::string message;

		[[nodiscard]] constexpr auto operator<=>(const Error& rhs) const noexcept = default;
	} error{};

	[[nodiscard]] bool operator==(const Node& rhs) const noexcept
	{
		// Compare owned data (not the child nodes, token or callee)
		return rhs.type == type
			&& rhs.value == value
			&& rhs.outType == outType
			&& rhs.nameSpace == nameSpace
			&& rhs.inType == inType
			&& rhs.api == api
			&& std::equal(rhs.parameters.begin(), rhs.parameters.end(), parameters.begin(), [](const auto& v1, const auto& v2) { return *v1 == *v2; });
	}

	[[nodiscard]] bool operator!=(const Node& rhs) const noexcept { return !operator==(rhs); }
	[[nodiscard]] operator bool() const noexcept { return type != Type::Empty; }

	[[nodiscard]] FunctionDeclaration ToFunctionDeclaration() const
	{
		return FunctionDeclaration
		{
			.name = value,
			.nameSpace = nameSpace,
			.outType = outType,
			.inType = inType,
			.parameters = parameters,
			.api = api,
		};
	}

	[[nodiscard]] std::string GetQualifiedName() const
	{
		std::ostringstream s;

		nameSpace.Print(s);
		s << value;

		return s.str();
	}

	[[nodiscard]] void Print(std::ostream& stream) const
	{
		stream << '[' << to_string(type) << ',';
		outType.Print(stream);
		stream << ',';
		token.Print(stream);
		stream << ',';
		nameSpace.Print(stream);
		stream << ',';
		inType.Print(stream);
		stream << ',' << '[';
		for (auto& param : parameters)
		{
			param->Print(stream);
			stream << ',';
		}
		stream << ']' << ',';
		api.Print(stream);
		stream << ']';
	}
};
