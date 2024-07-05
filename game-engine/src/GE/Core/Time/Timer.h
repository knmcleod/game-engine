#pragma once

#include <chrono>

namespace GE
{
	class Timer
	{
	public:
		Timer(const char* name)
			: m_Name(name), m_Stopped(false)
		{
			m_StartTime = std::chrono::high_resolution_clock::now();
		}

		virtual ~Timer()
		{
			if (!m_Stopped)
			{
				Stop();
			}
		}

		virtual void Stop()
		{
			auto endTime = std::chrono::high_resolution_clock::now();

			long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTime).time_since_epoch().count();
			long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();

			m_Stopped = true;

			float duration = (end - start) * 0.001f;
		}
	private:
		bool m_Stopped;
		const char* m_Name;
		std::chrono::time_point<std::chrono::steady_clock> m_StartTime;
	};
}
