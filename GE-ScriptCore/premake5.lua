project "GE-ScriptCore"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"
	staticruntime "on"
	
	targetdir ("%{wks.location}/editor/Resources/Scripts")
	objdir ("%{wks.location}/editor/Resources/Scripts/Intermediates")

	files
	{
		"Source/**.cs",
		"Properties/**.cs"
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
