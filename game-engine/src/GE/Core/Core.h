#pragma once

#include <memory>

#ifdef _WIN32
	#ifdef _WIN64
		#define GE_PLATFORM_WINDOWS
	#else
		#error "Windows x86 not supported!"
	#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>
	#if TARGET_IPHONE_SIMULATOR == 1
		#error "IOS simulator not supported!"
	#elif TARGET_OS_IPHONE == 1
		#define GE_PLATFORM_IOS
		#error "IOS not supported!"
	#elif TARGET_OS_MAC == 1
		#define GE_PLATFORM_MACOS
		#error "MacOS not supported!"
	#else
		#error "Unknown Apple platform!"
	#endif

#elif defined(__ANDRIOD__)
	#define GE_PLATFORM_ANDRIOD
	#error "Andriod not supported!"
#elif defined(__linux__)
	#define GE_PLATFORM_LINUX
	#error "Linux not supported!"
#else
	#error "Unknown Platform!"
#endif

#ifdef GE_PLATFORM_WINDOWS
	#define GE_DYNAMIC_LINK
	#ifdef GE_DYNAMIC_LINK
		#ifdef GE_BUILD_DLL
			#define GE_API __declspec(dllexport)
		#else
			#define GE_API __declspec(dllimport)
		#endif
	#else
		#define GE_API
	#endif
#else
	#error "GE only supports Windows!"
#endif // GE_PLATFORM_WINDOWS

#ifdef GE_DEBUG
	#define GE_ENABLE_ASSERTS
	#define GE_ENABLE_LOG
	#define GE_ENABLE_PROFILER

#endif // GE_DEBUG

#ifndef GE_DIST
	#define GE_ENABLE_VERIFY
#endif // GE_DEBUG

#ifdef GE_ENABLE_LOG
	#include "GE/Core/Debug/Log/Log.h"
	// Core Log Macros
	#define GE_CORE_TRACE(...)	::GE::Log::GetCoreLogger()->trace<>(__VA_ARGS__)
	#define GE_CORE_INFO(...)	::GE::Log::GetCoreLogger()->info<>(__VA_ARGS__)
	#define GE_CORE_WARN(...)	::GE::Log::GetCoreLogger()->warn<>(__VA_ARGS__)
	#define GE_CORE_ERROR(...)	::GE::Log::GetCoreLogger()->error<>(__VA_ARGS__)
	
	// Client Log Macros
	#define GE_TRACE(...)	::GE::Log::GetClientLogger()->trace<>(__VA_ARGS__)
	#define GE_INFO(...)	::GE::Log::GetClientLogger()->info<>(__VA_ARGS__)
	#define GE_WARN(...)	::GE::Log::GetClientLogger()->warn<>(__VA_ARGS__)
	#define GE_ERROR(...)	::GE::Log::GetClientLogger()->error<>(__VA_ARGS__)
#else
	// Core Log Macros
	#define GE_CORE_TRACE(...)
	#define GE_CORE_INFO(...)
	#define GE_CORE_WARN(...)
	#define GE_CORE_ERROR(...)
	
	// Client Log Macros
	#define GE_TRACE(...)
	#define GE_INFO(...)
	#define GE_WARN(...)
	#define GE_ERROR(...)
#endif

#ifdef GE_ENABLE_PROFILER
	#include "GE/Core/Debug/Profiler/Profiler.h"
	#define GE_PROFILE_BEGIN_SESSION(name, filepath) ::GE::Profiler::GetInstance()->BeginSession(name, filepath)
	#define GE_PROFILE_END_SESSION() ::GE::Profiler::GetInstance()->EndSession()
	#define GE_PROFILE_SCOPE(name) ::GE::ProfilerTimer timer##__LINE__(name);
	#define GE_PROFILE_FUNCTION() GE_PROFILE_SCOPE(__FUNCSIG__)
#else
	#define GE_PROFILE_BEGIN_SESSION(name, filepath)
	#define GE_PROFILE_END_SESSION()
	#define GE_PROFILE_SCOPE(name)
	#define GE_PROFILE_FUNCTION()
#endif

#ifdef GE_ENABLE_ASSERTS
	#define GE_ASSERT(x, ...) { if(!(x)) { GE_ERROR("Assertion Failed. {0}", __VA_ARGS__); __debugbreak(); } }
	#define GE_CORE_ASSERT(x, ...) { if(!(x)) { GE_CORE_ERROR("Assertion Failed. {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define GE_ASSERT(x, ...)
	#define GE_CORE_ASSERT(x, ...)
#endif // GE_ENABLE_ASSERTS

#ifdef GE_ENABLE_VERIFY
	#define GE_VERIFY(x, ...) { if(!(x)) { GE_ERROR("Verification Failed. {0}", __VA_ARGS__); __debugbreak(); } }
	#define GE_CORE_VERIFY(x, ...) { if(!(x)) { GE_CORE_ERROR("Verification Failed. {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define GE_VERIFY(x, ...)
	#define GE_CORE_VERIFY(x, ...)
#endif // GE_ENABLE_ASSERTS

#define BIT(x) (1 << x)

#define GE_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return fn(std::forward<decltype(args)>(args)...); }

namespace GE
{
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	/*
	* Makes no runtime check to ensure T is derived from U(base)
	*/
	template<class T, class U>
	Ref<T> static_ref_cast(const Ref<U>& r) noexcept
	{
		auto p = static_cast<typename std::shared_ptr<T>::element_type*>(r.get());
		return Ref<T>{r, p};
	}

	/*
	* Provides safe downcasts, i.e. base(U) to derived(T)
	*/
	template<class T, class U>
	constexpr Ref<T> dynamic_ref_cast(const Ref<U>& r) noexcept
	{
		if (auto p = dynamic_cast<typename std::shared_ptr<T>::element_type*>(r.get()))
			return Ref<T>{r, p};
		else
			return Ref<T>{};
	}

}