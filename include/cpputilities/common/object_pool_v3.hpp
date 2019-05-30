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
			//���������⣬��һ��shared_ptr���ü���Ϊ0��ʱ�򣬱����·ŵ�m_object_map�У�
			//����������shared_ptr�ڶ��α�ʹ�ö������ü���Ϊ0��ʱ������lambda���ʽ����û m_object_map.emplace(constructName, std::shared_ptr<T>(t));
			//û���ṩ�Զ���ɾ��������������ʱ�򲻻ᱻ���·Ż�m_object_map�С�
			//�����ԣ�����ֲ����˳�������
			class NonCopyable
			{
			protected:
				NonCopyable() = default;
				~NonCopyable() = default;
				// ���ø��ƹ���
				NonCopyable(const NonCopyable&) = delete;
				// ���ø�ֵ����
				NonCopyable& operator = (const NonCopyable&) = delete;
			};

			template<typename T>
			class ObjectPool :NonCopyable
			{
				template<typename...Args>
				using Constructor = std::function<std::shared_ptr<T>(Args...)>;

				const int MaxObjectNum = 10;

			public:
				// Ĭ�ϴ������ٸ�����
				template<typename... Args>
				void Init(size_t num, Args&&...args)
				{
					if (num <= 0 || num>MaxObjectNum)
					{
						throw std::logic_error("object num out of range.");
					}
					// ����������
					auto constructName = typeid(Constructor<Args...>).name();
					for (size_t i = 0; i<num; i++)
					{
						m_object_map.emplace(constructName, std::shared_ptr<T>(new T(std::forward<Args>(args)...), [this, constructName](T*p) {
							//ɾ�����в�ֱ��ɾ�����󣬶��ǻ��յ�������У��Թ��´�ʹ��
							m_object_map.emplace(std::move(constructName), std::shared_ptr<T>(p));
						}));

					}
				}
				// �Ӷ�����л�ȡһ������
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