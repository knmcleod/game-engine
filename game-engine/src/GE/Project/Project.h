#pragma once

#include "GE/Asset/Managers/AssetManager.h"

#include "GE/Core/Core.h"

#include <string>
#include <filesystem>

namespace GE
{
	struct ProjectSpecification
	{
		std::string Name = "New Project";
		std::filesystem::path ProjectPath;

		std::filesystem::path AssetPath;
		std::filesystem::path ScriptPath;

		UUID SceneHandle = 0;
	};

	class Project
	{
	public:
		inline ProjectSpecification& GetSpec() { return m_Spec; }

		inline std::shared_ptr<AssetManager> GetAssetManager() { return m_AssetManager; }

		inline static std::filesystem::path GetProjectPath()
		{
			GE_CORE_ASSERT(s_ActiveProject, "Cannot get Project Asset Path. No Active Project.");
			return s_ActiveProject->GetSpec().ProjectPath;
		}

		/*
		* Returns filesystem path from project dir to asset path
		* Example:
		*	output = projects/demo/assets
		*/
		inline static std::filesystem::path GetAssetPath()
		{
			GE_CORE_ASSERT(s_ActiveProject, "Cannot get Project Asset Path. No Active Project.");
			return GetProjectPath() / s_ActiveProject->GetSpec().AssetPath;
		}

		/*
		* Returns filesystem path from project dir to given asset path
		* Example:
		*	param path = scenes/demoExample.scene
		*	output = projects/demo/assets/path
		*/
		inline static std::filesystem::path GetPathToAsset(const std::filesystem::path& path)
		{
			GE_CORE_ASSERT(s_ActiveProject, "Cannot get Project Asset Path. No Active Project.");
			return s_ActiveProject->GetSpec().ProjectPath / s_ActiveProject->GetSpec().AssetPath / path;
		}

		inline static Ref<Project> GetActive()
		{
			return s_ActiveProject;
		}

		template<typename T>
		inline static Ref<T> NewAssetManager()
		{	
			Ref<T> newAssetManager = CreateRef<T>();
			GetActive()->m_AssetManager = newAssetManager;
			return std::static_pointer_cast<T>(GetActive()->m_AssetManager);
		}

		template<typename T>
		inline static Ref<T> GetAssetManager()
		{
			std::shared_ptr<AssetManager> assetManager = GetActive()->GetAssetManager();
			return std::static_pointer_cast<T>(assetManager);
		}

		template<typename T>
		inline static Ref<T> GetAsset(UUID handle)
		{
			Ref<Asset> asset = Project::GetActive()->GetAssetManager()->GetAsset(handle);
			return std::static_pointer_cast<T>(asset);
		}

		template<typename T>
		inline static Ref<T> GetAsset(const std::filesystem::path& filePath)
		{
			Ref<Asset> asset = Project::GetActive()->GetAssetManager()->GetAsset(filePath);
			return std::static_pointer_cast<T>(asset);
		}

		static Ref<Project> Load(const std::filesystem::path& path);
		static bool Save(const std::filesystem::path& path);
	private:
		ProjectSpecification m_Spec;
		Ref<AssetManager> m_AssetManager;

		inline static Ref<Project> s_ActiveProject;
	};

}