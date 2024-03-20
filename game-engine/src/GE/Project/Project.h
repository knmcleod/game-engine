#pragma once

#include "GE/Core/Core.h"

#include <string>
#include <filesystem>

namespace GE
{
	struct ProjectSpecification
	{
		std::string Name = "New Project";
		std::filesystem::path ProjectPath;

		std::filesystem::path ScenePath;

		std::filesystem::path AssetPath;
		std::filesystem::path ScriptPath;
	};

	class Project
	{
	public:
		ProjectSpecification& GetSpec() { return m_Spec; }
		
		static std::filesystem::path& GetProjectPath()
		{
			GE_CORE_ASSERT(s_ActiveProject, "Cannot get Project Asset Path. No Active Project.");
			return s_ActiveProject->m_Spec.ProjectPath;
		}

		/*
		* Returns filesystem path from project dir to asset path
		* Example:
		*	output = projects/demo/assets
		*/
		static std::filesystem::path GetAssetPath()
		{
			GE_CORE_ASSERT(s_ActiveProject, "Cannot get Project Asset Path. No Active Project.");
			return GetProjectPath() / s_ActiveProject->m_Spec.AssetPath;
		}

		/*
		* Returns filesystem path from project dir to given asset path
		* Example:
		*	param path = scenes/demoExample.scene
		*	output = projects/demo/assets/path
		*/
		static std::filesystem::path GetPathToAsset(const std::filesystem::path& path)
		{
			GE_CORE_ASSERT(s_ActiveProject, "Cannot get Project Asset Path. No Active Project.");
			return s_ActiveProject->m_Spec.ProjectPath / s_ActiveProject->m_Spec.AssetPath / path;
		}

		static Ref<Project> GetActive() { return s_ActiveProject; }

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool Save(const std::filesystem::path& path);
	private:
		ProjectSpecification m_Spec;

		inline static Ref<Project> s_ActiveProject;
	};

}