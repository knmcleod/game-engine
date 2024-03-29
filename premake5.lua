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
IncludeDir["box2d"] = "game-engine/vender/box2d/include"
IncludeDir["entt"] = "game-engine/vender/entt/single_include"
IncludeDir["filewatch"] = "game-engine/vender/filewatch"
IncludeDir["GLAD"] = "game-engine/vender/GLAD/include"
IncludeDir["glfw"] = "game-engine/vender/glfw/include"
IncludeDir["glm"] = "game-engine/vender/glm"
IncludeDir["ImGui"] = "game-engine/vender/imgui"
IncludeDir["mono"] = "game-engine/vender/mono/mono/msvc/include"
IncludeDir["msdf_atlas_gen"] = "game-engine/vender/msdf-atlas-gen"
IncludeDir["msdf_gen"] = "game-engine/vender/msdf-atlas-gen/msdfgen"
IncludeDir["spdlog"] = "game-engine/vender/spdlog/include"
IncludeDir["stb"] = "game-engine/vender/stb_image"
IncludeDir["yaml_cpp"] = "game-engine/vender/yaml-cpp/include"

Library = {}
Library["mono"] = "game-engine/vender/mono/mono/msvc/build/sgen/x64/lib/%{cfg.buildcfg}/libmono-static-sgen.lib"

-- Windows
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"

group "Dependencies"
	include "game-engine/vender/box2d"
	include "game-engine/vender/GLAD"
	include "game-engine/vender/glfw"
	include "game-engine/vender/ImGui"
	include "game-engine/vender/msdf-atlas-gen"
	include "game-engine/vender/msdf-atlas-gen/msdfgen"
	include "game-engine/vender/yaml-cpp"
group ""

group "Scripting"
	include "GE-ScriptCore"
group ""

project "game-engine"
	location "game-engine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "GE/GEpch.h"
	pchsource "%{prj.name}/src/GE/GEpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vender/glm/glm/**.hpp",
		"%{prj.name}/vender/glm/glm/**.inl",
		"%{prj.name}/vender/stb_image/stb_image.h",
		"%{prj.name}/vender/stb_image/stb_image.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.box2d}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.filewatch}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.glfw}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.msdf_atlas_gen}",
		"%{IncludeDir.msdf_gen}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.yaml_cpp}"
	}

	links
	{
		"box2d",
		"GLAD",
		"glfw",
		"ImGui",
		"msdf-atlas-gen",
		"msdfgen",
		"yaml-cpp",
		"opengl32.lib",
		"game-engine/vender/mono/mono/msvc/build/sgen/x64/lib/%{cfg.buildcfg}/libmono-static-sgen.lib"
	}

	filter "system:windows"
		systemversion "latest"

		links
		{
			"%{Library.WinSock}",
			"%{Library.WinMM}",
			"%{Library.WinVersion}",
			"%{Library.BCrypt}"
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
	staticruntime "off"

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
		"%{IncludeDir.entt}",
		"%{IncludeDir.filewatch}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.msdf_atlas_gen}",
		"%{IncludeDir.msdf_gen}",
		"%{IncludeDir.spdlog}"
	}

	links
	{
		"game-engine"
	}

	filter "system:windows"
		systemversion "latest"

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

project "demo"
	location "demo"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

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
		"%{IncludeDir.entt}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.spdlog}",
		
	}

	links
	{
		"game-engine"
	}

	filter "system:windows"
		systemversion "latest"

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
