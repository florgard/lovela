#include "pch.h"
#include "test.h"

TEST(ReturnValueTest, Success) {
	auto result = f_success(123);
	EXPECT_EQ(result.index(), 0);
	EXPECT_EQ(std::get<0>(result), 123);
}

TEST(ReturnValueTest, Failure) {
	auto result = f_error(123);
	EXPECT_EQ(result.index(), 1);
	EXPECT_EQ(std::get<1>(result).code, 1);
}
