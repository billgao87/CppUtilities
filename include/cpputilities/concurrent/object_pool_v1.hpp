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

			//������Ȼ��������ֻ�ܹ����޲ε��࣬�����в������಻��ʹ�ã����ң����������е�����virtual�������ƺ��������⣬�����ο�
			typedef int s32;
			typedef unsigned int u32;
			typedef char c8;

			//�������װ
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

			//�������װ
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


			//����ؽӿ�
			typedef interface IObjectPool
			{
				virtual void gc(bool IsForce) = 0;
			}*IObjectPoolPtr;

			//�����
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

				//�ⲿӦ�ö�ʱ��������������Զ������ڴ�
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

			//����ع����������Զ���һЩ�궨�壬ʵ�ֶ���������صĹ���
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

				//����ʵ��
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

				//��������
				void gc()
				{
					ScopeLock LockBody(mLock);
					for (auto i = mObjectPools.begin(); i != mObjectPools.end(); ++i)
					{
						i->second->gc(false);
					}
				}

				//�ر����õĹ��췽��
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
