#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace GE
{
	// Core Log Macros
#define GE_CORE_TRACE(...) GE::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define GE_CORE_INFO(...) GE::Log::GetCoreLogger()->info(__VA_ARGS__)
#define GE_CORE_WARN(...) GE::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define GE_CORE_ERROR(...) GE::Log::GetCoreLogger()->error(__VA_ARGS__)
#define GE_CORE_FATAL(...) GE::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client Log Macros
#define GE_TRACE(...) GE::Log::GetClientLogger()->trace(__VA_ARGS__)
#define GE_INFO(...) GE::Log::GetClientLogger()->info(__VA_ARGS__)
#define GET_WARN(...) GE::Log::GetClientLogger()->warn(__VA_ARGS__)
#define GE_ERROR(...) GE::Log::GetClientLogger()->error(__VA_ARGS__)
#define GE_FATAL(...) GE::Log::GetClientLogger()->fatal(__VA_ARGS__)

	class Log
	{
	public: 
		//	Initializes Log
		static void Init();

		//	Returns Core Logger
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }

		//	Returns Client Logger
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

