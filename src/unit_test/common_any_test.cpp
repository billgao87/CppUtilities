#include "stdafx.h"
#include "gtest/gtest.h"
#include "cpputilities/common/Any.h"

TEST(common_any_test, common_any_test)
{
	cpp_utilities::common::Any n;
	EXPECT_TRUE(n.IsNull()); //true
	std::string s1 = "hello";
	n = s1;
	n = "world";
	EXPECT_THROW(n.AnyCast<int>(), std::bad_cast); //can not cast int to string
	cpp_utilities::common::Any n1 = 1;
	EXPECT_TRUE(n1.Is<int>()); //true
}