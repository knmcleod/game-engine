project "GE-ScriptCore"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"
	staticruntime "on"

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.cs"
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
