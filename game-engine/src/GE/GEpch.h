#pragma once

#include <filesystem>
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream> 
#include <fstream>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>

//	-- Core	--
#include "GE/Core/Core.h"

#include "GE/Core/Debug/Instrumentor.h"

#ifdef GE_PLATFORM_WINDOWS
	#include <Windows.h>
	#include <commdlg.h>
#endif // GE_PLATFORM_WINDOWS

