#include "stdafx.h"
#include "gtest/gtest.h"
#include "cpputilities/common/object_pool_v1.hpp"
#include "cpputilities/common/object_pool_v2.hpp"
#include "cpputilities/common/object_pool_v3.hpp"

#include <memory>

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


TEST(object_pool_test, DISABLED_object_pool_v1_test)
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

TEST(object_pool_test, DISABLED_object_pool_v2_test)
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

	BigObject(const int& a, const int&b) {}
	void Print(const std::string& str)
	{
		std::cout << str << std::endl;
	}
};

void Print(std::shared_ptr<BigObject>p, const std::string& str)
{
	if (p != nullptr)
	{
		p->Print(str);
	}
}

TEST(object_pool_test, object_pool_v3_test)
{
	ObjectPool<BigObject> pool;
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