#include "stdafx.h"
#include "gtest/gtest.h"
#include "cpputilities/common/object_pool_v1.hpp"
#include "cpputilities/common/object_pool_v2.hpp"
#include "cpputilities/common/object_pool_v3.hpp"

#include <memory>
#include "cpputilities/common/object_pool.hpp"

using namespace cpp_utilities::common::object_pool;

class TestA
{
public:

	TestA()
	{
		printf("TestA() %p\n", this);
	}

	~TestA()
	{
		printf("~TestA() %p\n", this);
	}

	void DoSomething()
	{
		printf("DoSomething() %p\n", this);
	}
};


TEST(utilities_object_pool_test, DISABLED_object_pool_v1_test)
{
	ObjectPoolV1<TestA> TestPool(2);

	TestA *pTest1 = TestPool.New();
	TestA *pTest2 = TestPool.New();

	pTest1->DoSomething();
	pTest2->DoSomething();

	TestPool.Delete(pTest1);
	TestPool.Delete(pTest2);

	pTest1 = TestPool.New();
	pTest2 = TestPool.New();

	pTest1->DoSomething();
	pTest2->DoSomething();

	TestPool.Delete(pTest1);
	TestPool.Delete(pTest2);
}

TEST(utilities_object_pool_test, DISABLED_object_pool_v2_test)
{
	ObjectPoolV2<TestA> p;
	p.add(std::make_unique<TestA>());
	p.add(std::make_unique<TestA>());
	{
		auto t = p.get();
		p.get();
	}

	{
		p.get();
		p.get();
	}

	EXPECT_TRUE(p.size() == 2);
}

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
		std::cout << str << ": A = " << A << "; B = " << B << std::endl;
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

TEST(utilities_object_pool_test, DISABLED_object_pool_v3_test)
{
	ObjectPoolV3<BigObject> pool;
	pool.Init(2);
	{
		auto p = pool.Get();
		Print(p, "p");
		auto p2 = pool.Get();
		Print(p2, "p2");
		// 出了作用哉之后，对象池返回出来的对象又会自动回收
	}

	auto p = pool.Get();
	Print(p, "p");
	auto p2 = pool.Get();
	Print(p2, "p2");

	// 对象池支持重载构造函数
	pool.Init(2, 1);
	auto p4 = pool.Get<int>();
	Print(p4, "p4");
	pool.Init(2, 1, 2);
	auto p5 = pool.Get<int, int>();
	Print(p5, "p5");
}

TEST(utilities_object_pool_test, DISABLED_object_pool_v4_test)
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
}
