#ifndef CPP_UTILITIES_COMMON_OBEJCT_POOL_V2_H_
#define CPP_UTILITIES_COMMON_OBEJCT_POOL_V2_H_

#include <memory>
#include <vector>
#include <functional>

namespace cpputilities
{
	namespace common
	{
		namespace object_pool
		{
			//同步的对象池
			//自动释放对象到对象池中
			//添加已经构造好的对象
			template <class T>
			class ObjectPoolV2
			{
			public:
				using DeleterType = std::function<void(T*)>;

				void add(std::unique_ptr<T> t)
				{
					pool_.push_back(std::move(t));
				}

				std::unique_ptr<T, DeleterType> get()
				{
					if (pool_.empty())
					{
						throw std::logic_error("no more object");
					}

					//every time add custom deleter for default unique_ptr
					std::unique_ptr<T, DeleterType> ptr(pool_.back().release(), [this](T* t)
					{
						pool_.push_back(std::unique_ptr<T>(t));
					});

					pool_.pop_back();
					return std::move(ptr);
				}

				std::shared_ptr<T> get_shared()
				{
					if (pool_.empty())
					{
						throw std::logic_error("no more object");
					}

					auto pin = std::unique_ptr<T>(std::move(pool_.back()));
					pool_.pop_back();

					return std::shared_ptr<T>(pin.release(), [this](T* t)
					{
						pool_.push_back(std::unique_ptr<T>(t));
					});
				}

				bool empty() const
				{
					return pool_.empty();
				}

				size_t size() const
				{
					return pool_.size();
				}

			private:
				std::vector<std::unique_ptr<T>> pool_;
			};

		}	//!namespace object_pool
	}	//!namespace common
}	//!namespace cpputilities

#endif	//CPP_UTILITIES_COMMON_OBEJCT_POOL_V2_H_