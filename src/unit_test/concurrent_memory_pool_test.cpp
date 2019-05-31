#include "stdafx.h"
#include "gtest/gtest.h"

#include "cpputilities/concurrent/memory_pool.hpp"

using	 namespace  cpp_utilities::concurrent::memory_pool;

TEST(concurrent_memory_pool_test, memory_pool_v1_test)
{
	srand(time(nullptr));

	//分配400MB内存空间,每个块128bytes
	MemoryPool *pPool = new MemoryPool(1024 * 1024 * 400, 128);

	u32 AllocateSize = 0;
	u32 Tick = 0;
	u32 Cost = 0;
	for (u32 i = 0; i < 20; ++i)
	{
		Tick = GetTickCount();
		for (u32 j = 0; j < 25000; ++j)
		{
			AllocateSize = 64 + rand() % 1024;
			auto p = (char*)pPool->GetMemory(AllocateSize);
			pPool->ReleaseMemory(p);
		}
		Cost = GetTickCount() - Tick;
		printf("test of %u times pool cost %ums.\n", i + 1, Cost);

		Tick = GetTickCount();
		for (u32 j = 0; j < 25000; ++j)
		{
			AllocateSize = 64 + rand() % 1024;
			auto p = new char[AllocateSize];
			delete[] p;
		}
		Cost = GetTickCount() - Tick;
		printf("test of %u times new and delete cost %ums.\n", i + 1, Cost);
	}

	delete pPool;
}