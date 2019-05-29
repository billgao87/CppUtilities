#include "stdafx.h"
#include "gtest/gtest.h"
#include "cpputilities/common/object_pool_v1.hpp"
#include <memory>
#include "cpputilities/common/object_pool_v2.hpp"

using namespace cpputilities::common::object_pool;

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


TEST(object_pool_test, object_pool_v1_test)
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

TEST(object_pool_test, object_pool_v2_test)
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