#ifndef	CPP_UTILITIES_COMMON_STOPWATCH_H_
#define CPP_UTILITIES_COMMON_STOPWATCH_H_

#include <windows.h>
#include <chrono>

using namespace std::chrono;

namespace cpp_utilities
{
	namespace diagnose
	{
		inline namespace cpp11_duration
		{
			class StopWatch
			{
			public:
				StopWatch() :begin_(high_resolution_clock::now()) {};

				void Reset() { begin_ = high_resolution_clock::now(); }

				//ƒ¨»œ ‰≥ˆ∫¡√Î
				int64_t elapsed() const
				{
					return duration_cast<milliseconds>(high_resolution_clock::now() - begin_).count();
				}

				//Œ¢√Î
				int64_t elapsed_micro() const
				{
					return duration_cast<microseconds>(high_resolution_clock::now() - begin_).count();
				}

				//ƒ…√Î
				int64_t elapsed_nano() const
				{
					return duration_cast<nanoseconds>(high_resolution_clock::now() - begin_).count();
				}

				//√Î
				int64_t elapsed_seconds() const
				{
					return duration_cast<seconds>(high_resolution_clock::now() - begin_).count();
				}

				//∑÷
				int64_t elapsed_minutes() const
				{
					return duration_cast<minutes>(high_resolution_clock::now() - begin_).count();
				}

				// ±
				int64_t elapsed_hours() const
				{
					return duration_cast<hours>(high_resolution_clock::now() - begin_).count();
				}
			private:
				time_point<high_resolution_clock> begin_;
			};
		}

		namespace query_performance_timer
		{
			///////////////////////////////////////////////////////////////////////////////
			// Simple Stopwatch class. Use this for high resolution timing 
			// purposes (or, even low resolution timings)
			// Pretty self-explanitory.... 
			// Reset(), or GetElapsedSeconds().
			class StopWatch
			{
			public:
				StopWatch(void)	// Constructor
				{
					QueryPerformanceFrequency(&m_CounterFrequency);
					Start();
				}

				void Start()
				{
					QueryPerformanceCounter(&m_LastCount);
				}

				// Resets timer (difference) to zero
				void Reset(void)
				{
					QueryPerformanceCounter(&m_LastCount);
				}

				double elapsed() const
				{
					LARGE_INTEGER lCurrent;
					QueryPerformanceCounter(&lCurrent);
					return ((lCurrent.QuadPart - m_LastCount.QuadPart) * 1000) / double(m_CounterFrequency.QuadPart);
				}

				// Get elapsed time in seconds
				double elapsed_seconds() const
				{
					// Get the current count
					LARGE_INTEGER lCurrent;
					QueryPerformanceCounter(&lCurrent);
					return (lCurrent.QuadPart - m_LastCount.QuadPart) / double(m_CounterFrequency.QuadPart);
				}

			protected:
				LARGE_INTEGER m_CounterFrequency;
				LARGE_INTEGER m_LastCount;
			};

		}	//namespace query_performance_timer
	}	//!namespace diagnose

}	// namespace cpp_utilities

#endif	//CPP_UTILITIES_COMMON_STOPWATCH_H_
