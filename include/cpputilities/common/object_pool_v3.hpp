#ifndef CPP_UTILITIES_COMMON_OBJECT_POOL_V3_H_
#define CPP_UTILITIES_COMMON_OBJECT_POOL_V3_H_

#include <iostream>
#include <string>
#include <functional>
#include <memory>
#include <map>

namespace cpp_utilities
{
	namespace common
	{
		namespace object_pool
		{
			//此类有问题，第一次shared_ptr引用计数为0的时候，被重新放到m_object_map中，
			//但是如果这个shared_ptr第二次被使用而且引用计数为0的时候，由于lambda表达式里面没 m_object_map.emplace(constructName, std::shared_ptr<T>(t));
			//没有提供自定义删除操作，因此这个时候不会被重新放回m_object_map中。
			//经测试，会出现不能退出的问题
			class NonCopyable
			{
			protected:
				NonCopyable() = default;
				~NonCopyable() = default;
				// 禁用复制构造
				NonCopyable(const NonCopyable&) = delete;
				// 禁用赋值构造
				NonCopyable& operator = (const NonCopyable&) = delete;
			};

			template<typename T>
			class ObjectPool :NonCopyable
			{
				template<typename...Args>
				using Constructor = std::function<std::shared_ptr<T>(Args...)>;

				const int MaxObjectNum = 10;

			public:
				// 默认创建多少个对象
				template<typename... Args>
				void Init(size_t num, Args&&...args)
				{
					if (num <= 0 || num>MaxObjectNum)
					{
						throw std::logic_error("object num out of range.");
					}
					// 不区分引用
					auto constructName = typeid(Constructor<Args...>).name();
					for (size_t i = 0; i<num; i++)
					{
						m_object_map.emplace(constructName, std::shared_ptr<T>(new T(std::forward<Args>(args)...), [this, constructName](T*p) {
							//删除器中不直接删除对象，而是回收到对象池中，以供下次使用
							m_object_map.emplace(std::move(constructName), std::shared_ptr<T>(p));
						}));

					}
				}
				// 从对象池中获取一个对象
				template<typename... Args>
				std::shared_ptr<T> Get()
				{
					std::string constructName = typeid(Constructor<Args...>).name();
					auto range = m_object_map.equal_range(constructName);
					for (auto it = range.first; it != range.second; ++it)
					{
						auto ptr = it->second;
						m_object_map.erase(it);
						return ptr;
					}
					return nullptr;
				}
			private:
				std::multimap<std::string, std::shared_ptr<T>> m_object_map;
			};

		}	//!namespace object_pool
	}	//!namespace common
}	//!namespace cpp_utilities

#endif	//CPP_UTILITIES_COMMON_OBJECT_POOL_V3_H_