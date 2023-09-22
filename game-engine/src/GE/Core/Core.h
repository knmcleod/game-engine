#pragma once

#include <memory>

#include "GE/Core/Log/Log.h"

#ifdef _Win32
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
#endif

#ifdef GE_PLATFORM_WINDOWS
	#if GE_DYNAMIC_LINK
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
#endif // GE_DEBUG

#ifdef GE_ENABLE_ASSERTS
	#define GE_ASSERT(x, ...) { if(!(x)) { GE_ERROR("Assertion Failed{0}", __VA_ARGS__); __debugbreak(); } }
	#define GE_CORE_ASSERT(x, ...) { if(!(x)) { GE_ERROR("Assertion Failed{0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define GE_ASSERT(x, ...)
	#define GE_CORE_ASSERT(x, ...)
#endif // GE_ENABLE_ASSERTS

#define BIT(x) (1 << x)

#define GE_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace GE
{
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;
}