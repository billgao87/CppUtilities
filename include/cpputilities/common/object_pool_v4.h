#ifndef CPP_UTILITIES_COMMON_OBEJCT_POOL_V4_H_
#define CPP_UTILITIES_COMMON_OBEJCT_POOL_V4_H_

#include <list>

namespace cpp_utilities
{
	namespace common
	{
		namespace object_pool
		{

			template<typename Object>
			class ObjectPool
			{
			public:

				ObjectPool(size_t unSize) :
					m_unSize(unSize)
				{
					for (size_t unIdx = 0; unIdx < m_unSize; ++unIdx)
					{
						m_oPool.push_back(new Object());
					}
				}

				~ObjectPool()
				{
					typename std::list<Object *>::iterator oIt = m_oPool.begin();
					while (oIt != m_oPool.end())
					{
						delete (*oIt);
						++oIt;
					}
					m_unSize = 0;
				}

				Object * GetObject()
				{
					Object * pObj = NULL;
					if (0 == m_unSize)
					{
						pObj = new Object();
					}
					else
					{
						pObj = m_oPool.front();
						m_oPool.pop_front();
						--m_unSize;
					}

					return pObj;
				}

				void ReturnObject(Object * pObj)
				{
					m_oPool.push_back(pObj);
					++m_unSize;
				}

			private:

				size_t m_unSize;

				std::list<Object *> m_oPool;
			};

		}	//!namespace object_pool
	}	//!namespace common
}	//!namespace cpp_utilities


#endif	//CPP_UTILITIES_COMMON_OBEJCT_POOL_V4_H_