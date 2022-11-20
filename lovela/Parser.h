#pragma once
#include "ParserBase.h"

class Parser : public ParserBase
{
	friend class ParserFactory;

protected:
	Parser(TokenGenerator&& tokenGenerator) noexcept;

public:
	[[nodiscard]] NodeGenerator Parse() noexcept override;


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

	[[nodiscard]] TypeSpec ParseTypeSpec();
	[[nodiscard]] ParameterList ParseParameterList();

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

	[[nodiscard]] static constexpr TypeSpec GetPrimitiveTypeSpec(std::string_view value)
	{
		const auto pair = to_int<int64_t>(value);

		if (pair.first.has_value())
		{
			const auto i = pair.first.value();

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
		else if (pair.second.has_value())
		{
			const auto i = pair.second.value();

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

		return { .kind = TypeSpec::Kind::Invalid };
	}
};
