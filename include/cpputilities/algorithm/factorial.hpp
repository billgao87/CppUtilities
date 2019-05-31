#ifndef CPP_UTILITIES_ALGORITHM_FACTORIAL_H_
#define CPP_UTILITIES_ALGORITHM_FACTORIAL_H_
#include <algorithm>

namespace cpp_utilities
{
	namespace algorithm
	{
		// Returns n! (the factorial of n).  For negative n, n! is defined to be 1.
		inline int Factorial(int n) {
			int result = 1;
			for (int i = 1; i <= n; i++) {
				result *= i;
			}

			return result;
		}
	}	//!namespace algorithm

}	//!namespace cpp_utilities

#endif	//CPP_UTILITIES_ALGORITHM_FACTORIAL_H_
