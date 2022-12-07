#pragma once
#include "ILexer.h"
#include "ParserBase.h"
#include "ParserRegexes.h"

class Parser : public ParserBase
{
	friend class ParserTest;

public:
	[[nodiscard]] Generator Parse() noexcept override;

private:
	struct Context
	{
		std::shared_ptr<Context> parent;
		std::map<std::string, std::shared_ptr<FunctionDeclaration>> functionSymbols;
		std::map<std::string, std::shared_ptr<VariableDeclaration>> variableSymbols;
		TypeSpec inType{};

		[[nodiscard]] bool HasFunctionSymbol(const std::string& symbol) const;
		[[nodiscard]] bool HasVariableSymbol(const std::string& symbol) const;
		void AddFunctionSymbol(const std::string& symbol);
		void AddVariableSymbol(std::shared_ptr<VariableDeclaration> variable);
	};

	[[nodiscard]] std::unique_ptr<Node> ParseFunctionDeclaration(std::shared_ptr<Context> context);
	[[nodiscard]] std::unique_ptr<Node> ParseCompoundExpression(std::shared_ptr<Context> context);
	[[nodiscard]] std::unique_ptr<Node> ParseExpression(std::shared_ptr<Context> context);
	[[nodiscard]] std::unique_ptr<Node> ParseGroup(std::shared_ptr<Context> context);
	[[nodiscard]] std::unique_ptr<Node> ParseTuple(std::shared_ptr<Context> context);
	[[nodiscard]] std::unique_ptr<Node> ParseOperand(std::shared_ptr<Context> context);
	[[nodiscard]] std::unique_ptr<Node> ParseFunctionCall(std::shared_ptr<Context> context);
	[[nodiscard]] std::unique_ptr<Node> ParseBinaryOperation(std::shared_ptr<Context> context);
	[[nodiscard]] std::unique_ptr<Node> ParseVariableReference(std::shared_ptr<Context> context);

	[[nodiscard]] std::unique_ptr<Node> ReduceExpression(std::unique_ptr<Node>&& expression);

	[[nodiscard]] TypeSpec ParseTypeSpec();
	[[nodiscard]] ParameterList ParseParameterList();

	[[nodiscard]] static constexpr TypeSpec GetBuiltinTypeSpec(std::string_view value)
	{
		if (value.empty())
		{
			return { .kind = TypeSpec::Kind::Invalid };
		}

		TypeSpec t{ .kind = TypeSpec::Kind::Primitive };

		switch (value[0])
		{
		case 'i':
			t.primitive.signedType = true;
			break;

		case 'u':
			break;

		case 'f':
			t.primitive.floatType = true;
			break;

		default:
			return { .kind = TypeSpec::Kind::Invalid };
		}

		auto bits = to_int<char>(value.substr(1)).unsignedValue.value_or(0);
		switch (bits)
		{
		case 8:
			[[fallthrough]];

		case 16:
			if (t.primitive.floatType)
			{
				return { .kind = TypeSpec::Kind::Invalid };
			}
			[[fallthrough]];

		case 32:
			[[fallthrough]];

		case 64:
			t.primitive.bits = bits;
			break;

		default:
			return { .kind = TypeSpec::Kind::Invalid };
		}

		return t;
	}

	[[nodiscard]] static constexpr TypeSpec GetPrimitiveIntegerTypeSpec(std::string_view value)
	{
		const auto pair = to_int<int64_t>(value);

		if (pair.signedValue.has_value())
		{
			const auto i = pair.signedValue.value();

			if (i >= std::numeric_limits<int8_t>::min())
			{
				return { .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 8, .signedType = true} };
			}
			else if (i >= std::numeric_limits<int16_t>::min())
			{
				return { .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 16, .signedType = true} };
			}
			else if (i >= std::numeric_limits<int32_t>::min())
			{
				return { .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 32, .signedType = true} };
			}
			else if (i >= std::numeric_limits<int64_t>::min())
			{
				return { .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 64, .signedType = true} };
			}
		}
		else if (pair.unsignedValue.has_value())
		{
			const auto i = pair.unsignedValue.value();

			if (i <= std::numeric_limits<uint8_t>::max())
			{
				return { .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 8, .signedType = i <= static_cast<decltype(i)>(std::numeric_limits<int8_t>::max())}};
			}
			else if (i <= std::numeric_limits<uint16_t>::max())
			{
				return { .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 16, .signedType = i <= static_cast<decltype(i)>(std::numeric_limits<int16_t>::max())} };
			}
			else if (i <= std::numeric_limits<uint32_t>::max())
			{
				return { .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 32, .signedType = i <= static_cast<decltype(i)>(std::numeric_limits<int32_t>::max())} };
			}
			else if (i <= std::numeric_limits<uint64_t>::max())
			{
				return { .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 64, .signedType = i <= static_cast<decltype(i)>(std::numeric_limits<int64_t>::max())} };
			}
		}

		return { .kind = TypeSpec::Kind::Invalid };
	}

	[[nodiscard]] TypeSpec GetPrimitiveDecimalTypeSpec(const std::string& value);

	ParserRegexes regexes;
};

using RangeParser = BasicRangeParser<Parser>;

inline auto operator>>(ILexer::Generator&& tokens, RangeParser& parser)
{
	parser.Initialize(std::move(tokens));
	return parser.Parse();
}

inline std::vector<Node>& operator>>(IParser::Generator&& input, std::vector<Node>& output)
{
	output = std::move(to_vector(std::move(input)));
	return output;
}
