#include "stdafx.h"
#include "gtest/gtest.h"
#include <memory>
#include "cpputilities/concurrent/object_pool.hpp"
#include <future>

using namespace cpp_utilities::concurrent::object_pool;

struct BigObject
{
	BigObject() {}
	BigObject(int a) {}

	BigObject(const int& a, const int&b)
	{
		A = a;
		B = b;
	}
	void Print(const std::string& str)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		std::cout << str << ": A = " << A << "; B = " << B << "; thread id = "<<std::this_thread::get_id()<< std::endl;
	}

	int A = 0;
	int B = 0;
};


static void Print(std::shared_ptr<BigObject>p, const std::string& str)
{
	if (p != nullptr)
	{
		p->Print(str);
	}
}

TEST(concurrent_object_pool_test, object_pool_test)
{
	ObjectPool<BigObject> pool;
	pool.Init(2);
	{
		auto p = pool.get_shared();
		Print(p, "p");
		auto p2 = pool.get_shared();
		Print(p2, "p2");
		// 出了作用哉之后，对象池返回出来的对象又会自动回收
	}
	printf("Pool size = %d \n", static_cast<int>(pool.size()));
	EXPECT_TRUE(pool.size() == 2);

	{
		auto p = pool.get_shared();
		Print(p, "p");
		auto p2 = pool.get_shared();
		Print(p2, "p2");

		auto p3 = pool.get_shared(1, 2);
		Print(p3, "p3");
		printf("Pool size = %d \n", static_cast<int>(pool.size()));
		EXPECT_TRUE(pool.size() == 9);
	}

	printf("Pool size = %d \n", static_cast<int>(pool.size()));
	EXPECT_TRUE(pool.size() == 12);

	std::async(std::launch::async,[&]()
	{
		auto p = pool.get_shared();
		Print(p, "p4");
		auto p2 = pool.get_shared();
		Print(p2, "p5");
	});

	{
		auto p = pool.get_shared();
		Print(p, "p6");
		auto p2 = pool.get_shared();
		Print(p2, "p7");
	}

	printf("Pool size = %d \n", static_cast<int>(pool.size()));
	EXPECT_TRUE(pool.size() == 12);
}