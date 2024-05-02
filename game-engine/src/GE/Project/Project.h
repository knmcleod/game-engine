#pragma once

#include "GE/Asset/AssetManager.h"

#include "GE/Core/Core.h"

#include <string>
#include <filesystem>

namespace GE
{
	class Project
	{
		friend class ProjectSerializer;
	public:
		struct Config
		{
			std::string Name = "New Project";
			std::filesystem::path ProjectPath;

			std::filesystem::path AssetPath;
			std::filesystem::path ScriptPath;

			UUID SceneHandle = 0;
		};

		inline static const UUID& GetSceneHandle() { return s_ActiveProject->m_Config.SceneHandle; }

		inline static Ref<Project> GetActive() { return s_ActiveProject; }

		inline static const std::filesystem::path GetProjectPath()
		{
			GE_CORE_ASSERT(s_ActiveProject, "Cannot get Project Asset Path. No Active Project.");
			return std::filesystem::path(s_ActiveProject->m_Config.ProjectPath);
		}

		/*
		* Returns filesystem path from project dir to asset path
		* Example:
		*	output = projects/demo/assets
		*/
		inline static const std::filesystem::path GetAssetPath()
		{
			GE_CORE_ASSERT(s_ActiveProject, "Cannot get Project Asset Path. No Active Project.");
			return std::filesystem::path(GetProjectPath() / s_ActiveProject->m_Config.AssetPath);
		}

		/*
		* Returns filesystem path from project dir to given asset path
		* Example:
		*	param path = scenes/demoExample.scene
		*	output = projects/demo/assets/path
		*/
		inline static const std::filesystem::path GetPathToAsset(const std::filesystem::path& path)
		{
			GE_CORE_ASSERT(s_ActiveProject, "Cannot get Project Asset Path. No Active Project.");
			return std::filesystem::path(s_ActiveProject->m_Config.ProjectPath / s_ActiveProject->m_Config.AssetPath / path);
		}

		template<typename T>
		inline static Ref<T> NewAssetManager()
		{	
			Ref<T> assetManager = CreateRef<T>();
			s_ActiveProject->m_AssetManager = assetManager;
			return std::static_pointer_cast<T>(s_ActiveProject->m_AssetManager);
		}

		template<typename T>
		inline static Ref<T> GetAssetManager()
		{
			return std::static_pointer_cast<T>(s_ActiveProject->m_AssetManager);
		}

		template<typename T>
		inline static Ref<T> GetAsset(UUID handle)
		{
			Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
			return std::static_pointer_cast<T>(asset);
		}

		template<typename T>
		static Ref<T> GetAsset(const std::filesystem::path& filePath)
		{
			Ref<Asset> asset = Project::GetAssetManager<EditorAssetManager>()->GetAsset(filePath);
			return std::static_pointer_cast<T>(asset);
		}

		static Ref<Project> Load(const std::filesystem::path& path);
		static bool Save(const std::filesystem::path& path);

		static inline const Config& GetConfig() { return s_ActiveProject->m_Config; }
		static inline Ref<AssetManager> GetAssetManager() { return s_ActiveProject->m_AssetManager; }

	private:
		Config m_Config = Config();
		Ref<AssetManager> m_AssetManager;

		inline static Ref<Project> s_ActiveProject;
	};

}