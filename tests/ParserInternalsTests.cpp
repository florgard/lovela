#include "pch.h"

class ParserTest
{
public:
	// Internal function forwarding

	[[nodiscard]] static constexpr TypeSpec GetBuiltinTypeSpec(std::string_view value)
	{
		return Parser::GetBuiltinTypeSpec(value);
	}

	[[nodiscard]] static constexpr TypeSpec GetPrimitiveIntegerTypeSpec(std::string_view value)
	{
		return Parser::GetPrimitiveIntegerTypeSpec(value);
	}
};

using namespace boost::ut;

suite parser_GetBuiltinTypeSpec_tests = [] {
	static_assert(ParserTest::GetBuiltinTypeSpec("i8") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 8, .signedType = true} });
	static_assert(ParserTest::GetBuiltinTypeSpec("i16") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 16, .signedType = true} });
	static_assert(ParserTest::GetBuiltinTypeSpec("i32") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 32, .signedType = true} });
	static_assert(ParserTest::GetBuiltinTypeSpec("i64") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 64, .signedType = true} });
	static_assert(ParserTest::GetBuiltinTypeSpec("u8") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 8, .signedType = false} });
	static_assert(ParserTest::GetBuiltinTypeSpec("u16") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 16, .signedType = false} });
	static_assert(ParserTest::GetBuiltinTypeSpec("u32") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 32, .signedType = false} });
	static_assert(ParserTest::GetBuiltinTypeSpec("u64") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 64, .signedType = false} });
	static_assert(ParserTest::GetBuiltinTypeSpec("f32") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 32, .floatType = true} });
	static_assert(ParserTest::GetBuiltinTypeSpec("f64") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 64, .floatType = true} });

	static_assert(ParserTest::GetBuiltinTypeSpec("") == TypeSpec{ .kind = TypeSpec::Kind::Invalid });
	static_assert(ParserTest::GetBuiltinTypeSpec(" ") == TypeSpec{ .kind = TypeSpec::Kind::Invalid });
	static_assert(ParserTest::GetBuiltinTypeSpec("a") == TypeSpec{ .kind = TypeSpec::Kind::Invalid });
	static_assert(ParserTest::GetBuiltinTypeSpec("a8") == TypeSpec{ .kind = TypeSpec::Kind::Invalid });
	static_assert(ParserTest::GetBuiltinTypeSpec("i9") == TypeSpec{ .kind = TypeSpec::Kind::Invalid });
	static_assert(ParserTest::GetBuiltinTypeSpec("f8") == TypeSpec{ .kind = TypeSpec::Kind::Invalid });
	static_assert(ParserTest::GetBuiltinTypeSpec("f16") == TypeSpec{ .kind = TypeSpec::Kind::Invalid });
};

suite parser_GetPrimitiveTypeSpec_tests = [] {
	// i8
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("0") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 8, .signedType = true} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("1") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 8, .signedType = true} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("+1") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 8, .signedType = true} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("-1") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 8, .signedType = true} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("127") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 8, .signedType = true} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("-128") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 8, .signedType = true} });

	// u8
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("128") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 8, .signedType = false} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("255") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 8, .signedType = false} });

	// i16
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("-129") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 16, .signedType = true} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("256") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 16, .signedType = true} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("32767") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 16, .signedType = true} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("-32768") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 16, .signedType = true} });

	// u16
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("32768") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 16, .signedType = false} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("65535") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 16, .signedType = false} });

	// i32
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("-32769") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 32, .signedType = true} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("65536") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 32, .signedType = true} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("2147483647") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 32, .signedType = true} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("-2147483648") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 32, .signedType = true} });

	// u32
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("2147483648") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 32, .signedType = false} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("4294967295") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 32, .signedType = false} });

	// i64
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("-2147483649") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 64, .signedType = true} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("4294967296") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 64, .signedType = true} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("9223372036854775807") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 64, .signedType = true} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("-9223372036854775808") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 64, .signedType = true} });

	// u64
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("9223372036854775808") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 64, .signedType = false} });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("18446744073709551615") == TypeSpec{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 64, .signedType = false} });

	// Overflow
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("-9223372036854775809") == TypeSpec{ .kind = TypeSpec::Kind::Invalid });
	static_assert(ParserTest::GetPrimitiveIntegerTypeSpec("18446744073709551616") == TypeSpec{ .kind = TypeSpec::Kind::Invalid });
};
