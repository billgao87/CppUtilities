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
#include <vector>

namespace cpp_utilities
{
	namespace concurrent
	{
		namespace memory_pool
		{

			//������Ȼ��������ֻ�ܹ����޲ε��࣬�����в������಻��ʹ�ã����ң����������е�����virtual�������ƺ��������⣬�����ο�
			typedef int s32;
			typedef unsigned int u32;
			typedef char c8;
			typedef long long s64;
			typedef unsigned long long u64;
			typedef void* LPVOID;
			typedef unsigned char* LPBYTE;

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
				LockObject(const LockObject &other) {}
				LockObject& operator = (const LockObject &other) {}
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
				ScopeLock& operator = (const ScopeLock &other) {}
			private:
				CRITICAL_SECTION &mlock;
			};

			//�ڴ��ṹ
			typedef struct MemoryChunk
			{
				u64 mID;			//��ID
				LPVOID mpData;		//�������ṩ���ⲿ�����ڴ�
				u64 mDataSize;		//�������ݿ�Ĵ�С
				u64 mUsedSize;		//ʹ�����ݿ��С
				MemoryChunk *mpNext;//��һ����
			}*MemoryChunkPtr;


			//�ڴ������ʵ��
			class MemoryPool
			{
			public:
				MemoryPool(u64 MemoryBlockSize, u64 MemoryChunkSize)
				{
					mTotalMemoryPoolSize = 0;
					mUsedMemorySize = 0;
					mFreeMemorySize = 0;
					mChunkIDPool = 0;

					mpBeginChunk = nullptr;
					mpNearFreeChunk = nullptr;
					mpEndChunk = nullptr;

					mMemoryChunkSize = MemoryChunkSize;
					_AllocateMemory(MemoryBlockSize);
				}

				~MemoryPool()
				{
					ScopeLock LockBody(mLock);
					_Destory();
				}

				//��ȡ�ڴ�
				template<typename T>
				T* GetMemory(u64 Count)
				{
					if (Count <= 0)
					{
						assert(false);
						return nullptr;
					}
					auto p = static_cast<T*>(GetMemory(Count * sizeof(T)));
					if (nullptr == p)
					{
						assert(false);
						return nullptr;
					}

					//���ù��췽��
					for (auto i = 0; i < Count; ++i)
					{
						new (p + i)T();
					}
					return p;
				}

				LPVOID GetMemory(u64 MemoryBlockSize)
				{
					ScopeLock LockBody(mLock);

					//���������ʵ�ֵ
					const auto NewMemoryBlockSize = _AdjustMemoryBlockSize(MemoryBlockSize);

					MemoryChunkPtr pBeginChunk = nullptr;
					MemoryChunkPtr pEndChunk = nullptr;

					//�ҵ����ʵĿ�
					if (false == _FindAllocateChunk(NewMemoryBlockSize, pBeginChunk, pEndChunk))
					{
						if (false == _AllocateMemory(NewMemoryBlockSize))
						{
							assert(false);
							return nullptr;
						}
						if (false == _FindAllocateChunk(NewMemoryBlockSize, pBeginChunk, pEndChunk))
						{
							assert(false);
							return nullptr;
						}
					}

					//�������ʹ�����,ע��pEndChunk�ǿ�����( [ pBeginChunk, pEndChunk ) )
					_AdjustChunkData(pBeginChunk, pEndChunk, MemoryBlockSize);

					//�����ڴ�ʹ�ô�С
					mUsedMemorySize += NewMemoryBlockSize;
					mFreeMemorySize = mTotalMemoryPoolSize - mUsedMemorySize;

					//��������Ŀ��п�����´η��������
					if (mpNearFreeChunk == pBeginChunk)
					{
						mpNearFreeChunk = pEndChunk;
					}

					//��¼��������
					AllocChunk AC = { pBeginChunk, pEndChunk, NewMemoryBlockSize };
					mAllocBlocks.insert(std::make_pair(pBeginChunk->mpData, AC));
					return pBeginChunk->mpData;
				}
				template<typename T>
				void ReleaseMemory(T* p, u64 count)
				{
					if (nullptr == p || count <= 0)
					{
						assert(false);
						return;
					}
					ScopeLock LockBody(mLock);

					const auto Search = mAllocBlocks.find(p);
					if (Search == mAllocBlocks.end())
					{
						assert(false);
						return;
					}

					//��������
					for (auto i = 0; i < count; ++i)
					{
						(p + i)->~T();
					}

					//����������п�λ��
					if (mpNearFreeChunk->mID > Search->second.mBeginChunk->mID)
					{
						mpNearFreeChunk = Search->second.mBeginChunk;
					}

					//�黹�ڴ�
					do
					{
						Search->second.mBeginChunk->mUsedSize = 0;
						Search->second.mBeginChunk = Search->second.mBeginChunk->mpNext;
					} while (Search->second.mBeginChunk != Search->second.mEndChunk);
					mAllocBlocks.erase(p);
				}

				void ReleaseMemory(LPVOID p)
				{
					if (nullptr == p)
					{
						assert(false);
						return;
					}

					ScopeLock LockBody(mLock);

					const auto Search = mAllocBlocks.find(p);
					if (Search == mAllocBlocks.end())
					{
						assert(false);
						return;
					}

					//����������п�λ��
					if (mpNearFreeChunk->mID > Search->second.mBeginChunk->mID)
					{
						mpNearFreeChunk = Search->second.mBeginChunk;
					}

					do
					{
						Search->second.mBeginChunk->mUsedSize = 0;
						Search->second.mBeginChunk = Search->second.mBeginChunk->mpNext;
					} while (Search->second.mBeginChunk != Search->second.mEndChunk);

					mAllocBlocks.erase(p);
				}

				//�ر����õķ���
			private:
				MemoryPool() {}
				MemoryPool(const MemoryPool &other)	{}
				MemoryPool& operator = (const MemoryPool &other) {}

				bool _AllocateMemory(u64 MemoryBlockSize)
				{
					//���������ֵ
					const auto NewMemoryBlockSize = _AdjustMemoryBlockSize(MemoryBlockSize);

					auto pNewMemoryBlock = malloc(NewMemoryBlockSize);
					if (nullptr == pNewMemoryBlock)
					{
						assert(false);
						return false;
					}

					//���������
					const auto NewChunkCount = NewMemoryBlockSize / mMemoryChunkSize;
					auto pNewMemoryChunk = malloc(NewChunkCount * sizeof(MemoryChunk));
					if (nullptr == pNewMemoryChunk)
					{
						assert(false);
						return false;
					}
					mTotalMemoryPoolSize += NewMemoryBlockSize;
					if (false == _CreateNewChunkLink(pNewMemoryBlock, pNewMemoryChunk, NewChunkCount))
					{
						assert(false);
						return false;
					}
					return true;
				}

				u64 _AdjustMemoryBlockSize(u64 MemoryBlockSize)
				{
					const auto Mod = MemoryBlockSize % mMemoryChunkSize;
					return Mod <= 0 ? MemoryBlockSize : MemoryBlockSize + mMemoryChunkSize - Mod;
				}

				//�����µĿ�����
				bool _CreateNewChunkLink(LPVOID pNewMemoryBlock, LPVOID pNewMemoryChunk, u64 NewChunkCount)
				{
					MemoryChunkPtr pNewChunk = nullptr;
					u64 MemoryOffset = 0;
					for (auto i = 0; i < NewChunkCount; ++i)
					{
						pNewChunk = static_cast<MemoryChunkPtr>(pNewMemoryChunk) + i;
						if (nullptr == mpBeginChunk)
						{
							mpBeginChunk =
								mpNearFreeChunk =
								mpEndChunk = pNewChunk;
						}
						else
						{
							mpEndChunk->mpNext = pNewChunk;
							mpEndChunk = pNewChunk;
						}

						MemoryOffset = i * mMemoryChunkSize;

						mpEndChunk->mpData = static_cast<LPBYTE>(pNewMemoryBlock) + MemoryOffset;
						mpEndChunk->mDataSize = mTotalMemoryPoolSize - MemoryOffset;
						mpEndChunk->mUsedSize = 0;
						mpEndChunk->mID = ++mChunkIDPool;
					}
					mpEndChunk->mpNext = nullptr;

					//��¼�׵�ַ�����ͷ�
					HeadAddress Address = { pNewMemoryChunk, pNewMemoryBlock };
					mHeadAddresses.push_back(Address);
					return true;
				}

				bool _FindAllocateChunk(u64 MemoryBlockSize, MemoryChunkPtr &pBeginChunk, MemoryChunkPtr &pEndChunk)
				{
					auto pTemp = mpNearFreeChunk;
					while (pTemp != nullptr && (pTemp->mDataSize < MemoryBlockSize || pTemp->mUsedSize > 0))
					{
						pTemp = pTemp->mpNext;
					}
					if (nullptr == pTemp)
					{
						return false;
					}
					pBeginChunk = pTemp;
					pEndChunk = pBeginChunk->mpNext;
					for (u64 i = mMemoryChunkSize; i < MemoryBlockSize; i += mMemoryChunkSize)
					{
						pEndChunk = pEndChunk->mpNext;
					}
					return true;
				}

				void _AdjustChunkData(MemoryChunkPtr pBeginChunk, MemoryChunkPtr pEndChunk, u64 MemoryBlockSize)
				{
					u64 Counter = 0;
					for (auto i = pBeginChunk; i != pEndChunk; i = i->mpNext)
					{
						Counter += mMemoryChunkSize;
						if (Counter > MemoryBlockSize)
						{
							i->mUsedSize = MemoryBlockSize - mMemoryChunkSize;
						}
						else
						{
							i->mUsedSize = mMemoryChunkSize;
						}
					}
				}

				void _Destory()
				{
					for (auto i = mHeadAddresses.begin(); i != mHeadAddresses.end(); ++i)
					{
						free(i->m1);
						free(i->m2);
					}
					mHeadAddresses.clear();
					mAllocBlocks.clear();
				}

			private:

				MemoryChunkPtr mpBeginChunk;
				MemoryChunkPtr mpNearFreeChunk;
				MemoryChunkPtr mpEndChunk;

				u64 mChunkIDPool;
				u64 mTotalMemoryPoolSize;
				u64 mUsedMemorySize;
				u64 mFreeMemorySize;
				u64 mMemoryChunkSize;
				struct HeadAddress
				{
					LPVOID m1;
					LPVOID m2;
				};
				std::vector<HeadAddress> mHeadAddresses;

				struct AllocChunk
				{
					MemoryChunkPtr mBeginChunk;
					MemoryChunkPtr mEndChunk;
					u64 mMemoryBlockSize;
				};
				std::map<LPVOID, AllocChunk> mAllocBlocks;

				LockObject mLock;
			};

		}	//!namespace memory_pool
	}	//!namespace concurrent
}	//!namespace cpp_utilities

#endif	//CPP_UTILITIES_CONCURRENT_OBJECT_POOL_V1_H_
