// unit_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "gtest/gtest.h"

int main(int argc, char **argv)
{
	printf("Running main() from CTUnitTest.cpp\n");
	testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();
	system("pause");
	return 0;
}


