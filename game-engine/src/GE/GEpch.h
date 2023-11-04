#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream> 
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>

//	-- Core	--
#include "GE/Core/Core.h"

//	-- Debug	--
#include "GE/Core/Debug/Instrumentor.h"
#include "GE/Core/Debug/Log/Log.h"

//	-- Entry Point	--
#include "GE/Core/EntryPoint.h"

#ifdef GE_PLATFORM_WINDOWS
	#include <Windows.h>
#endif // GE_PLATFORM_WINDOWS

