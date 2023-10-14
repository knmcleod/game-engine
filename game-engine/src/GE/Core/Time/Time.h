#pragma once

#include <chrono>
#include <iostream>

namespace GE
{
	class Timestep
	{
	public:
		Timestep(float time = 0.0f) : m_Time(time)
		{
		};

		operator float() const { return m_Time;  }

		float GetSeconds() const { return m_Time; }
		float GetMilliSeconds() const { return m_Time * 1000.0f; }
	private: 
		float m_Time;
	};

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

