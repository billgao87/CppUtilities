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
			//��Ҫ��ʱ���Զ������ڴ棻ʵ�ַ���һ����ڴ棬freepool����������ݣ�chunkpool�������ڴ棬�Ӵ���ڴ���ֱ�Ӵ�������
			//T�������޲ι��캯��
			//���T�а�������ָ�룬
			template<typename T>
			class ObjectPoolV1
			{
			public:
				//��ʼ�����С�����Ԥ�ȹ�������С�����ܴﵽ���
				//��:ֻ����һ���ڴ�ֻ�ͷ�һ���ڴ�
				ObjectPoolV1(size_t chunk_size = 32);

				virtual ~ObjectPoolV1();

				//���ÿ��С
				void SetChunkSize(size_t chunk_size);

				//ģ��new�ؼ��ַ����ڴ沢���ù��캯��
				T* New();

				//ģ��delete�ؼ����ͷ��ڴ������������
				void Delete(T *pT);

			private:
				//������С
				void AllocateChunk(size_t chuck_size);

				//�ͷſ�
				void ReleaseChunk();
			private:
				//���г�
				std::queue<T*> free_pool_;
				//���
				std::vector<T*> chunk_pool_;
				//��ǰ���С
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
				//���ȷ������ڴ棬Ȼ�󽫵�ַ�ӵ�free_pool�У�����ڴ�ŵ�chunkpool�б���
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