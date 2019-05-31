#ifndef CPP_UTILITIES_COMMON_ANY_H_
#define CPP_UTILITIES_COMMON_ANY_H_

#include <iostream>
#include <string>
#include <memory>
#include <typeindex>

namespace cpp_utilities
{
	namespace common
	{
		/**
		 * \brief any能容纳所有类型的数据，因此当赋值给any时，需要将值的类型擦除才行，即以一种通用的方式保存所有类型的数据。
		 * 这里可以通过继承去擦除类型，基类是不含模板参数的，派生类中才有模板参数，这个模板参数类型正是赋值的类型，在赋值时，
		 * 将创建的派生类对象赋值给基类指针，基类的派生类中携带了数据类型，基类只是原始数据的一个占位符，通过多态，
		 * 它擦除了原始数据类型，因此，任何数据类型都可以赋值给他，从而实现了能存放所有类型数据的目标。当取数据时需要向下转换
		 * 成派生类型来获取原始数据，当转换失败时打印详情，并抛出异常。由于any赋值时需要创建一个派生类对象，所以还需要管理该对象
		 * 的生命周期，这里用unique_ptr智能指针去管理对象的生命周期。
		 * https://www.cnblogs.com/qicosmos/p/3420095.html
		 */
		struct Any
		{
			Any(void) : m_tpIndex(std::type_index(typeid(void))) {}
			Any(const Any& that) : m_ptr(that.Clone()), m_tpIndex(that.m_tpIndex) {}
			Any(Any && that) noexcept : m_ptr(std::move(that.m_ptr)), m_tpIndex(that.m_tpIndex) {}

			//创建智能指针时，对于一般的类型，通过std::decay来移除引用和cv符，从而获取原始类型
			template<typename U, class = typename std::enable_if<!std::is_same<typename std::decay<U>::type, Any>::value, U>::type> Any(U && value) : m_ptr(new Derived < typename std::decay<U>::type>(std::forward<U>(value))),
				m_tpIndex(std::type_index(typeid(typename std::decay<U>::type))) {}

			bool IsNull() const { return !bool(m_ptr); }

			template<class U> bool Is() const
			{
				return m_tpIndex == std::type_index(typeid(U));
			}

			//将Any转换为实际的类型
			template<class U>
			U& AnyCast()
			{
				if (!Is<U>())
				{
					std::cout << "can not cast " << typeid(U).name() << " to " << m_tpIndex.name() << std::endl;
					throw std::bad_cast();
				}

				auto derived = dynamic_cast<Derived<U>*> (m_ptr.get());
				return derived->m_value;
			}

			Any& operator=(const Any& a)
			{
				if (m_ptr == a.m_ptr)
					return *this;

				m_ptr = a.Clone();
				m_tpIndex = a.m_tpIndex;
				return *this;
			}

		private:
			struct Base;
			typedef std::unique_ptr<Base> BasePtr;

			struct Base
			{
				virtual ~Base() {}
				virtual BasePtr Clone() const = 0;
			};

			template<typename T>
			struct Derived : Base
			{
				template<typename U>
				Derived(U && value) : m_value(std::forward<U>(value)) { }

				BasePtr Clone() const
				{
					return BasePtr(new Derived<T>(m_value));
				}

				T m_value;
			};

			BasePtr Clone() const
			{
				if (m_ptr != nullptr)
					return m_ptr->Clone();

				return nullptr;
			}

			BasePtr m_ptr;
			std::type_index m_tpIndex;
		};

	}	//!namespace common
}	//!namespace cpp_utilities

#endif	//CPP_UTILITIES_COMMON_ANY_H_