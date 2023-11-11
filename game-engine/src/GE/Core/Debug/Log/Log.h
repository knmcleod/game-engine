#pragma once
// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace GE
{
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
