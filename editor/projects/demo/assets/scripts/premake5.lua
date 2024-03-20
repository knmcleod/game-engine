local GERootDir = os.getenv("GE_DIR")

workspace "demo"
	architecture "x86_64"
	startproject "demo"
	
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}

group "GE"
	include (GERootDir .. "/GE-ScriptCore")
group ""

project "demo"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"
	staticruntime "on"
	
	targetdir ("%{wks.location}/Resources/Binaries")
	objdir ("%{wks.location}/Resources/Intermediates")

	files
	{
		"Source/**.cs",
		"Properties/**.cs"
	}

	links
	{
		"GE-ScriptCore"
	}

	filter "configurations:Debug"
		optimize "Off"
		symbols "Default"

	filter "configurations:Release"
		optimize "On"
		symbols "Default"

	filter "configurations:Dist"
		optimize "On"
		symbols "Default"
