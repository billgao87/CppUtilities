#ifndef CPP_UTILITIES_CONCURRENT_OBJECT_POOL_V1_H_
#define CPP_UTILITIES_CONCURRENT_OBJECT_POOL_V1_H_

#include "Windows.h"
#include <iostream>
#include <map>
#include <string>
#include <assert.h>
#include <deque>
#include <map>
#include <set>

namespace cpp_utilities
{
	namespace concurrent
	{
		namespace object_pool
		{

			//此类虽然不错，但是只能构造无参的类，对于有参数的类不能使用，而且，析构函数中调用了virtual函数，似乎存在问题，仅供参考
			typedef int s32;
			typedef unsigned int u32;
			typedef char c8;

			//锁对象封装
			class LockObject
			{
			public:
				LockObject()
				{
					InitializeCriticalSection(&mLock);
				}
				~LockObject()
				{
					DeleteCriticalSection(&mLock);
				}

				void Lock()
				{
					EnterCriticalSection(&mLock);
				}

				void UnLock()
				{
					LeaveCriticalSection(&mLock);
				}

				bool TryLock()
				{
					return TryEnterCriticalSection(&mLock);
				}
			private:
				LockObject(const LockObject &other)
				{}
				LockObject& operator = (const LockObject &other)
				{}
			private:
				CRITICAL_SECTION mLock;
			};

			//锁区域封装
			class ScopeLock
			{
			public:

				ScopeLock(CRITICAL_SECTION &lock)
					:mlock(lock)
				{
					EnterCriticalSection(&mlock);
				}
				ScopeLock(LockObject &lock)
					:mlock(reinterpret_cast<CRITICAL_SECTION&>(lock))
				{
					EnterCriticalSection(&mlock);
				}
				~ScopeLock()
				{
					LeaveCriticalSection(&mlock);
				}

			private:
				ScopeLock(const ScopeLock &other)
					:mlock(other.mlock)
				{}
				ScopeLock& operator = (const ScopeLock &other)
				{}
			private:
				CRITICAL_SECTION &mlock;
			};


			//对象池接口
			typedef interface IObjectPool
			{
				virtual void gc(bool IsForce) = 0;
			}*IObjectPoolPtr;

			//对象池
			template<typename T>
			class ObjectPool : public IObjectPool
			{
			public:

				ObjectPool(u32 ChunkSize = 32)
				{
					mChunkSize = ChunkSize;
				}

				virtual ~ObjectPool()
				{
					gc(true);
				}

				T* New()
				{
					ScopeLock LockBody(mLock);
					if (mFreeBlocks.empty())
					{
						_Malloc();
					}
					if (mFreeBlocks.empty())
					{
						assert(false);
						return nullptr;
					}
					auto p = mFreeBlocks.front();
					mFreeBlocks.pop_front();
					new(p)T();
					return p;
				}

				void Delete(T* p)
				{
					ScopeLock LockBody(mLock);
					p->~T();
					mFreeBlocks.push_back(p);
				}

				//外部应该定时调用这个函数，自动回收内存
				virtual void gc(bool IsForce)
				{
					ScopeLock LockBody(mLock);

					auto IsCangc = true;

					std::set<T*> FreeBlocks(mFreeBlocks.begin(), mFreeBlocks.end());

					T* pT = nullptr;

					for (auto i = mChunks.begin(); i != mChunks.end();)
					{
						for (auto j = 0; j < i->first; ++j)
						{
							pT = i->second + j;
							if (FreeBlocks.end() == FreeBlocks.find(pT))
							{
								if (IsForce)
								{
									pT->~T();
								}
								else
								{
									IsCangc = false;
									break;
								}
							}
						}
						if (IsCangc)
						{
							for (auto j = 0; j < i->first; ++j)
							{
								FreeBlocks.erase(j + i->second);
							}
							free(i->second);
							i = mChunks.erase(i);
							mChunkSize >>= 1;
						}
						else
						{
							IsCangc = true;
							++i;
						}
					}
					mFreeBlocks.assign(FreeBlocks.begin(), FreeBlocks.end());
				}

			private:

				void _Malloc()
				{
					const auto pMemory = reinterpret_cast<T*>(malloc(mChunkSize * sizeof(T)));
					if (nullptr == pMemory)
					{
						assert(false);
						return;
					}

					const auto Insert = mChunks.insert(std::make_pair(mChunkSize, pMemory));
					if (false == Insert.second)
					{
						assert(false);
						free(pMemory);
						return;
					}

					for (auto i = 0; i < mChunkSize; ++i)
					{
						mFreeBlocks.push_back(pMemory + i);
					}
					mChunkSize <<= 1;
				}

			private:

				std::map<u32, T*> mChunks;
				std::deque<T*> mFreeBlocks;
				u32 mChunkSize;
				LockObject mLock;
			};

			//对象池工厂，可以自定义一些宏定义，实现对整个对象池的管理
			class ObjectPoolFactory
			{
			public:

				~ObjectPoolFactory()
				{
					ScopeLock LockBody(mLock);
					for (auto i = mObjectPools.begin(); i != mObjectPools.end(); ++i)
					{
						delete i->second;
						i->second = nullptr;
					}
					mObjectPools.clear();
				}

				//单件实例
				static ObjectPoolFactory& GetInstance()
				{
					static ObjectPoolFactory Instance;
					return Instance;
				}

				template< typename T >
				ObjectPool<T>* CreateObjectPool(c8 *pName)
				{
					if (nullptr == pName)
					{
						assert(false);
						return nullptr;
					}
					ScopeLock LockmObjectPools(mLock);
					const auto Search = mObjectPools.find(pName);
					if (mObjectPools.end() == Search)
					{
						const auto Insert = mObjectPools.insert(std::make_pair(pName, new ObjectPool<T>()));
						if (false == Insert.second)
						{
							assert(false);
							return nullptr;
						}
						return (ObjectPool<T>*)Insert.first->second;
					}
					return (ObjectPool<T>*)Search->second;
				}

				//垃圾回收
				void gc()
				{
					ScopeLock LockBody(mLock);
					for (auto i = mObjectPools.begin(); i != mObjectPools.end(); ++i)
					{
						i->second->gc(false);
					}
				}

				//关闭无用的构造方法
			private:
				ObjectPoolFactory()
				{}

				ObjectPoolFactory(const ObjectPoolFactory &other)
				{}

				ObjectPoolFactory& operator =(const ObjectPoolFactory &other)
				{}

			private:

				LockObject mLock;
				std::map<std::string, IObjectPoolPtr> mObjectPools;
			};
		}
	}	//!namespace concurrent
}	//!namespace cpp_utilities



#endif	//CPP_UTILITIES_CONCURRENT_OBJECT_POOL_V1_H_
