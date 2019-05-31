#ifndef CPP_UTILITIES_ALGORITHM_PRIME_H_
#define CPP_UTILITIES_ALGORITHM_PRIME_H_
#include <algorithm>

namespace cpp_utilities
{
	namespace algorithm
	{
		// Returns true if n is a prime number.
		inline bool IsPrime(int n) {
			// Trivial case 1: small numbers
			if (n <= 1) return false;

			// Trivial case 2: even numbers
			if (n % 2 == 0) return n == 2;

			// Now, we have that n is odd and n >= 3.

			// Try to divide n by every odd number i, starting from 3
			for (int i = 3; ; i += 2) {
				// We only have to try i up to the squre root of n
				if (i > n / i) break;

				// Now, we have i <= n/i < n.
				// If n is divisible by i, n is not prime.
				if (n % i == 0) return false;
			}

			// n has no integer factor in the range (1, n), and thus is prime.
			return true;
		}

		// The prime table interface.
		class PrimeTable {
		public:
			virtual ~PrimeTable() {}

			// Returns true if n is a prime number.
			virtual bool IsPrime(int n) const = 0;

			// Returns the smallest prime number greater than p; or returns -1
			// if the next prime is beyond the capacity of the table.
			virtual int GetNextPrime(int p) const = 0;
		};

		// Implementation #1 calculates the primes on-the-fly.
		class OnTheFlyPrimeTable : public PrimeTable {
		public:
			virtual bool IsPrime(int n) const {
				if (n <= 1) return false;

				for (int i = 2; i*i <= n; i++) {
					// n is divisible by an integer other than 1 and itself.
					if ((n % i) == 0) return false;
				}

				return true;
			}

			virtual int GetNextPrime(int p) const {
				for (int n = p + 1; n > 0; n++) {
					if (IsPrime(n)) return n;
				}

				return -1;
			}
		};

		// Implementation #2 pre-calculates the primes and stores the result
		// in an array.
		class PreCalculatedPrimeTable : public PrimeTable 
		{
		public:
			// 'max' specifies the maximum number the prime table holds.
			explicit PreCalculatedPrimeTable(int max)
				: is_prime_size_(max + 1), is_prime_(new bool[max + 1])
			{
				CalculatePrimesUpTo(max);
			}
			virtual ~PreCalculatedPrimeTable() { delete[] is_prime_; }

			virtual bool IsPrime(int n) const 
			{
				return 0 <= n && n < is_prime_size_ && is_prime_[n];
			}

			virtual int GetNextPrime(int p) const 
			{
				for (int n = p + 1; n < is_prime_size_; n++) 
				{
					if (is_prime_[n]) return n;
				}

				return -1;
			}

		private:
			void CalculatePrimesUpTo(int max) 
			{
				std::fill(is_prime_, is_prime_ + is_prime_size_, true);
				is_prime_[0] = is_prime_[1] = false;

				for (int i = 2; i <= max; i++) 
				{
					if (!is_prime_[i]) continue;

					// Marks all multiples of i (except i itself) as non-prime.
					for (int j = 2 * i; j <= max; j += i) 
					{
						is_prime_[j] = false;
					}
				}
			}

			const int is_prime_size_;
			bool* const is_prime_;

			// Disables compiler warning "assignment operator could not be generated."
			void operator=(const PreCalculatedPrimeTable& rhs);
		};

	}	//!namespace algorithm

}	//!namespace cpp_utilities

#endif	//CPP_UTILITIES_ALGORITHM_PRIME_H_