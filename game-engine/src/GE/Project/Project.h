#pragma once

#include "GE/Asset/AssetManager.h"
#include "GE/Asset/RuntimeAssetManager.h"
#include "GE/Core/Core.h"

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
			uint32_t Width = 1280, Height = 720;

			UUID SceneHandle = 0;
			
			// TODO : EditorProject(?)
			std::filesystem::path ProjectPath = std::filesystem::path();
			std::filesystem::path AssetPath = std::filesystem::path();
			std::filesystem::path ScriptPath = std::filesystem::path();
		};

		/*
		* Types of Asset Managers
		*	1: Runtime, read from binary file
		*	2: Editor, read from text file
		*/
		template<typename T>
		inline static Ref<T> NewAssetManager()
		{	
			Ref<T> assetManager = CreateRef<T>();
			s_ActiveProject->m_AssetManager = assetManager;
			return static_ref_cast<T, AssetManager>(s_ActiveProject->m_AssetManager);
		}

		template<typename T>
		inline static Ref<T> GetAssetManager()
		{
			return static_ref_cast<T, AssetManager>(s_ActiveProject->m_AssetManager);
		}

		template<typename T>
		inline static Ref<T> GetAsset(UUID handle)
		{
			Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
			return static_ref_cast<T, Asset>(asset);
		}
		
		template<typename T>
		inline static Ref<T> GetAssetAs(Ref<Asset> asset)
		{
			return static_ref_cast<T, Asset>(asset);
		}

		template<typename T>
		inline static Ref<T> GetCopy(UUID handle)
		{
			Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
			return GetCopy<T>(asset);
		}
		
		template<typename T>
		inline static Ref<T> GetCopy(Ref<Asset> asset)
		{
			return static_ref_cast<T, Asset>(asset->GetCopy());
		}

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

		/*
		* Returns filesystem path from project dir to given script asset path
		* Example:
		*	param path = Player/Player.cs
		*	output = projects/demo/assets/scripts/Source/path
		*/
		inline static const std::filesystem::path GetPathToScriptAsset(const std::filesystem::path& path)
		{
			GE_CORE_ASSERT(s_ActiveProject, "Cannot get Project Asset Path. No Active Project.");
			return std::filesystem::path(GetPathToAsset("scripts/Source") / path);
		}

		inline static void SetViewport(uint32_t width, uint32_t height) { s_ActiveProject->m_Config.Width = width; s_ActiveProject->m_Config.Height = height; }
		inline static const Config& GetConfig() { return s_ActiveProject->m_Config; }
		inline static const uint32_t& GetWidth() { return s_ActiveProject->m_Config.Width; }
		inline static const uint32_t& GetHeight() { return s_ActiveProject->m_Config.Height; }

		inline static const UUID& GetSceneHandle() { return s_ActiveProject->m_Config.SceneHandle; }
		inline static Ref<AssetManager> GetAssetManager() { return s_ActiveProject->m_AssetManager; }

		inline static Ref<Project> GetActive() { return s_ActiveProject; }
		inline static Ref<Project> New();

		static bool Load(const std::filesystem::path& path);
		static bool Save(const std::filesystem::path& path);

	private: 
		Config m_Config = Config();
		Ref<AssetManager> m_AssetManager;

		static Ref<Project> s_ActiveProject;
	};

}