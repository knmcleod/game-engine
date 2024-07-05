#pragma once

#include <string>
#include <algorithm>
#include <fstream>

#include <thread>

#include "GE/Core/Time/Timer.h"

namespace GE
{
	class Profiler
	{
	public:
		struct Result
		{
			std::string Name;
			long long Start, End;
			uint32_t ThreadID;
		};

		struct Session
		{
			std::string Name;
		};

		Profiler() : m_CurrentSession(nullptr), m_ProfileCount(0)
		{
			s_Instance = this;
		}

		static Profiler* GetInstance()
		{
			if (s_Instance)
				return s_Instance;
			else
				new Profiler();
			return s_Instance;
		}

		static void BeginSession(const std::string& name, const std::string& filepath = "results.json")
		{
			s_Instance->m_OutputStream.open(filepath);
			WriteHeader();
			s_Instance->m_CurrentSession = new Session{ name };
		}

		static void EndSession()
		{
			WriteFooter();
			s_Instance->m_OutputStream.close();
			delete s_Instance->m_CurrentSession;
			s_Instance->m_CurrentSession = nullptr;
			s_Instance->m_ProfileCount = 0;
		}

		static void WriteProfile(const Result& result)
		{
			if (s_Instance->m_ProfileCount++ > 0)
				s_Instance->m_OutputStream << ",";

			std::string name = result.Name;
			std::replace(name.begin(), name.end(), '"', '\'');

			s_Instance->m_OutputStream << "{";
			s_Instance->m_OutputStream << "\"cat\":\"function\",";
			s_Instance->m_OutputStream << "\"dur\":" << (result.End - result.Start) << ',';
			s_Instance->m_OutputStream << "\"name\":\"" << name << "\",";
			s_Instance->m_OutputStream << "\"ph\":\"X\",";
			s_Instance->m_OutputStream << "\"pid\":0,";
			s_Instance->m_OutputStream << "\"tid\":" << result.ThreadID << ",";
			s_Instance->m_OutputStream << "\"ts\":" << result.Start;
			s_Instance->m_OutputStream << "}";

			s_Instance->m_OutputStream.flush();

		}

	private:
		static void WriteHeader()
		{
			s_Instance->m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
			s_Instance->m_OutputStream.flush();
		}

		static void WriteFooter()
		{
			s_Instance->m_OutputStream << "]}";
			s_Instance->m_OutputStream.flush();
		}

	private:
		static Profiler* s_Instance;

		Session* m_CurrentSession;
		std::ofstream m_OutputStream;
		int m_ProfileCount;
	};

	class ProfilerTimer : public Timer
	{
	private:
		bool m_Stopped;
		const char* m_Name;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;

	public:
		ProfilerTimer(const char* name) : Timer(name), m_Name(name), m_Stopped(false)
		{
			m_StartTime = std::chrono::high_resolution_clock::now();
		}

		virtual ~ProfilerTimer()
		{
			if (!m_Stopped)
			{
				Stop();
			}
		};

		virtual void Stop() override
		{
			auto endTimepoint = std::chrono::high_resolution_clock::now();

			long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTime).time_since_epoch().count();
			long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

			uint32_t threadID = (uint32_t)std::hash<std::thread::id>{}(std::this_thread::get_id());
			Profiler::GetInstance()->WriteProfile({ m_Name, start, end, threadID });
			m_Stopped = true;
		}
	};
}