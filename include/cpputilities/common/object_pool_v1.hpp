#ifndef CPP_UTILITIES_COMMON_OBEJCT_POOL_V1_H_
#define CPP_UTILITIES_COMMON_OBEJCT_POOL_V1_H_
#include <queue>
#include <cassert>

namespace cpp_utilities
{
	namespace common
	{
		namespace object_pool
		{
			//需要的时候自动分配内存；实现分配一大块内存，freepool保存空闲数据，chunkpool保存大块内存，从大块内存上直接创建对象，
			//T必须有无参构造函数
			//如果T中包含数组指针，
			template<typename T>
			class ObjectPoolV1
			{
			public:
				//初始化块大小如果能预先估算出块大小则性能达到最大
				//即:只分配一次内存只释放一次内存
				ObjectPoolV1(size_t chunk_size = 32);

				virtual ~ObjectPoolV1();

				//设置块大小
				void SetChunkSize(size_t chunk_size);

				//模拟new关键字分配内存并调用构造函数
				T* New();

				//模拟delete关键字释放内存调用析构函数
				void Delete(T *pT);

			private:
				//分配块大小
				void AllocateChunk(size_t chuck_size);

				//释放块
				void ReleaseChunk();
			private:
				//空闲池
				std::queue<T*> free_pool_;
				//块池
				std::vector<T*> chunk_pool_;
				//当前块大小
				size_t chunk_size_;
			};

			template <typename T>
			ObjectPoolV1<T>::ObjectPoolV1(size_t chunk_size)
				:chunk_size_(chunk_size)
			{
			}

			template <typename T>
			ObjectPoolV1<T>::~ObjectPoolV1()
			{
				ReleaseChunk();
			}

			template <typename T>
			void ObjectPoolV1<T>::SetChunkSize(size_t chunk_size)
			{
				if (0 == chunk_size)
				{
					assert(false);
					return;
				}

				chunk_size_ = chunk_size;
				if (free_pool_.empty())
				{
					return;
				}

				if (free_pool_.size() < chunk_size_)
				{
					AllocateChunk(chunk_size_ - free_pool_.size());
				}
			}

			template <typename T>
			T* ObjectPoolV1<T>::New()
			{
				if (free_pool_.empty())
				{
					AllocateChunk(chunk_size_);
				}
				if (free_pool_.empty())
				{
					assert(false);
					return nullptr;
				}

				T* pT = free_pool_.front();
				free_pool_.pop();
				new(pT)T();
				return pT;
			}

			template <typename T>
			void ObjectPoolV1<T>::Delete(T* pT)
			{
				if (nullptr == pT)
				{
					assert(false);
					return;
				}
				pT->~T();
				free_pool_.push(pT);
			}

			template <typename T>
			void ObjectPoolV1<T>::AllocateChunk(size_t chuck_size)
			{
				if (0 == chuck_size)
				{
					assert(false);
					return;
				}
				//首先分配大块内存，然后将地址加到free_pool中，大块内存放到chunkpool中保存
				T* pT = reinterpret_cast<T*>(malloc(chuck_size * sizeof(T)));
				if (nullptr == pT)
				{
					assert(false);
					return;
				}

				for (size_t i = 0; i < chuck_size; ++i)
				{
					free_pool_.push(pT + i);
				}
				chunk_pool_.push_back(pT);
			}

			template <typename T>
			void ObjectPoolV1<T>::ReleaseChunk()
			{
				for (size_t i = 0; i < chunk_pool_.size(); i++)
				{
					free(chunk_pool_[i]);
					chunk_pool_[i] = nullptr;
				}
				chunk_pool_.clear();
			}
		}	//!namespace object_pool
	}	//!namespace common
}	//!namespace cpp_utilities


#endif	//CPP_UTILITIES_COMMON_OBEJCT_POOL_V1_H_