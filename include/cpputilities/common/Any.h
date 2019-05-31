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
		 * \brief any�������������͵����ݣ���˵���ֵ��anyʱ����Ҫ��ֵ�����Ͳ������У�����һ��ͨ�õķ�ʽ�����������͵����ݡ�
		 * �������ͨ���̳�ȥ�������ͣ������ǲ���ģ������ģ��������в���ģ����������ģ������������Ǹ�ֵ�����ͣ��ڸ�ֵʱ��
		 * �����������������ֵ������ָ�룬�������������Я�����������ͣ�����ֻ��ԭʼ���ݵ�һ��ռλ����ͨ����̬��
		 * ��������ԭʼ�������ͣ���ˣ��κ��������Ͷ����Ը�ֵ�������Ӷ�ʵ�����ܴ�������������ݵ�Ŀ�ꡣ��ȡ����ʱ��Ҫ����ת��
		 * ��������������ȡԭʼ���ݣ���ת��ʧ��ʱ��ӡ���飬���׳��쳣������any��ֵʱ��Ҫ����һ��������������Ի���Ҫ����ö���
		 * ���������ڣ�������unique_ptr����ָ��ȥ���������������ڡ�
		 * https://www.cnblogs.com/qicosmos/p/3420095.html
		 */
		struct Any
		{
			Any(void) : m_tpIndex(std::type_index(typeid(void))) {}
			Any(const Any& that) : m_ptr(that.Clone()), m_tpIndex(that.m_tpIndex) {}
			Any(Any && that) noexcept : m_ptr(std::move(that.m_ptr)), m_tpIndex(that.m_tpIndex) {}

			//��������ָ��ʱ������һ������ͣ�ͨ��std::decay���Ƴ����ú�cv�����Ӷ���ȡԭʼ����
			template<typename U, class = typename std::enable_if<!std::is_same<typename std::decay<U>::type, Any>::value, U>::type> Any(U && value) : m_ptr(new Derived < typename std::decay<U>::type>(std::forward<U>(value))),
				m_tpIndex(std::type_index(typeid(typename std::decay<U>::type))) {}

			bool IsNull() const { return !bool(m_ptr); }

			template<class U> bool Is() const
			{
				return m_tpIndex == std::type_index(typeid(U));
			}

			//��Anyת��Ϊʵ�ʵ�����
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