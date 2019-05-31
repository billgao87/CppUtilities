#ifndef CPP_UTILITIES_CONCURRENT_OBEJCT_POOL_H_
#define CPP_UTILITIES_CONCURRENT_OBEJCT_POOL_H_

#include <memory>
#include <vector>
#include <functional>
#include <mutex>

namespace cpp_utilities
{
	namespace concurrent
	{
		namespace object_pool
		{
			//同步的对象池
			//自动释放对象到对象池中
			//添加已经构造好的对象
			//在V2的基础上进行更改
			template <class T>
			class ObjectPool
			{
			public:
				using DeleterType = std::function<void(T*)>;

				const int ExpandSize = 10;

				// 默认创建多少个对象
				template<typename... Args>
				void Init(size_t num, Args...args)
				{
					if (num <= 0)
					{
						throw std::logic_error("object num out of range.");
					}
					allocate_chunk<Args...>(num, args...);
				}

				void add(std::unique_ptr<T> t)
				{
					pool_.push_back(std::move(t));
				}

				std::unique_ptr<T, DeleterType> get()
				{
					std::lock_guard<std::mutex> lk(get_release_lock_);

					if (pool_.empty())
					{
						throw std::logic_error("no more object");
					}

					//every time add custom deleter for default unique_ptr
					std::unique_ptr<T, DeleterType> ptr(pool_.back().release(), [this](T* t)
					{
						std::lock_guard<std::mutex> lk(get_release_lock_);
						pool_.push_back(std::unique_ptr<T>(t));
					});

					pool_.pop_back();
					return std::move(ptr);
				}

				std::shared_ptr<T> get_shared()
				{
					std::lock_guard<std::mutex> lk(get_release_lock_);

					if (pool_.empty())
					{
						throw std::logic_error("no more object");
					}

					auto pin = std::unique_ptr<T>(std::move(pool_.back()));
					pool_.pop_back();

					return std::shared_ptr<T>(pin.release(), [this](T* t)
					{
						std::lock_guard<std::mutex> lk(get_release_lock_);
						pool_.push_back(std::unique_ptr<T>(t));
					});
				}

				template<typename... Args>
				std::shared_ptr<T> get_shared(Args...args)
				{
					std::lock_guard<std::mutex> lk(get_release_lock_);

					if (pool_.empty())
					{
						allocate_chunk<Args...>(ExpandSize, args...);
					}

					auto pin = std::unique_ptr<T>(std::move(pool_.back()));
					pool_.pop_back();

					return std::shared_ptr<T>(pin.release(), [this](T* t)
					{
						std::lock_guard<std::mutex> lk(get_release_lock_);
						pool_.push_back(std::unique_ptr<T>(t));
					});
				}

				bool empty() const
				{
					std::lock_guard<std::mutex> lk(get_release_lock_);
					return pool_.empty();
				}

				size_t size() const
				{
					std::lock_guard<std::mutex> lk(get_release_lock_);
					return pool_.size();
				}

			private:
				template<typename... Args>
				void allocate_chunk(size_t size, Args...args)
				{
					for (size_t i = 0; i < size; i++)
					{
						add(std::unique_ptr<T>(new T(args...)));
					}
				}

			private:
				std::vector<std::unique_ptr<T>> pool_;
				mutable std::mutex get_release_lock_;
			};

		}	//!namespace object_pool
	}	//!namespace concurrent
}	//!namespace cpp_utilities

#endif	//CPP_UTILITIES_CONCURRENT_OBEJCT_POOL_H_

