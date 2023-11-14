workspace "game-engine"
	architecture "x86_64"
	startproject "editor"
	
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["glfw"] = "game-engine/vender/glfw/include"
IncludeDir["GLAD"] = "game-engine/vender/GLAD/include"
IncludeDir["ImGui"] = "game-engine/vender/imgui"
IncludeDir["glm"] = "game-engine/vender/glm"
IncludeDir["spdlog"] = "game-engine/vender/spdlog/include"
IncludeDir["stb"] = "game-engine/vender/stb"
IncludeDir["entt"] = "game-engine/vender/entt/single_include"
IncludeDir["yaml_cpp"] = "game-engine/vender/yaml-cpp/include"

group "Dependencies"
	include "game-engine/vender/glfw"
	include "game-engine/vender/GLAD"
	include "game-engine/vender/imgui"
	include "game-engine/vender/yaml-cpp"
group ""

project "game-engine"
	location "game-engine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "GE/GEpch.h"
	pchsource "%{prj.name}/src/GE/GEpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vender/stb/stb_image.h",
		"%{prj.name}/vender/stb/stb_image.cpp",
		"%{prj.name}/vender/glm/glm/**.hpp",
		"%{prj.name}/vender/glm/glm/**.inl"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.glfw}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}"
	}

	links
	{
		"glfw",
		"GLAD",
		"ImGui",
		"yaml-cpp",
		"opengl32.lib",
		"dwmapi.lib"
	}

	filter "system:windows"
		systemversion "latest"
		defines
		{
			"GE_PLATFORM_WINDOWS",
			"GE_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines "GE_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "GE_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "GE_DIST"
		runtime "Release"
		optimize "On"

project "editor"
	location "editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"game-engine/src",
		"game-engine/vender",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}"
	}

	links
	{
		"game-engine"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"GE_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "GE_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "GE_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "GE_DIST"
		runtime "Release"
		optimize "On"

project "sandbox"
	location "sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"game-engine/src",
		"game-engine/vender",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}"
	}

	links
	{
		"game-engine"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"GE_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "GE_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "GE_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "GE_DIST"
		runtime "Release"
		optimize "On"
