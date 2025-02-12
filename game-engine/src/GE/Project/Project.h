#pragma once

#include "GE/Asset/Assets/Scene/Scene.h"
#include "GE/Asset/AssetManager.h"

#include "GE/Core/Core.h"
#include "GE/Core/Events/Event.h"
#include "GE/Core/Time/Timestep.h"

#include <filesystem>

namespace GE
{
	class Project
	{
		friend class ProjectSerializer;
	public:
		struct Config
		{
			friend class Project;
			friend class ProjectSerializer;
		private:
			// Application/Project Name
			std::string Name = std::string("NewProject");
			// Application/Project Width & Height
			uint32_t Width = 1280, Height = 720;

			// TODO : Add IsFullScreen

			UUID SceneHandle = 0;
			Ref<Scene> RuntimeScene = nullptr;

			std::map<uint32_t, std::string> AllTags = std::map<uint32_t, std::string>();

			// projects/projName
			std::filesystem::path ProjectPath = std::filesystem::path();
			// assets
			std::filesystem::path AssetPath = std::filesystem::path();
			// scripts/src, relative to assets
			std::filesystem::path ScriptAssetPath = std::filesystem::path();
			// scripts/bin/GE-ScriptCore.dll, relative to assets
			std::filesystem::path ScriptCorePath = std::filesystem::path();
			// scripts/bin/projName.dll, relative to assets
			std::filesystem::path ScriptAppPath = std::filesystem::path();

			void Resize(const uint32_t& w, const uint32_t& h) 
			{
				if ((w == 0 || h == 0) || (w == Width && h == Height)) return; 
				Width = w; Height = h;
			}
		};

		inline static const Config& GetConfig() { return s_ActiveProject->m_Config; }
		inline static const std::string& GetName() { return s_ActiveProject->m_Config.Name; }
		inline static const uint32_t& GetWidth() { return s_ActiveProject->m_Config.Width; }
		inline static const uint32_t& GetHeight() { return s_ActiveProject->m_Config.Height; }

		// Returns RuntimeScene(temp/copy Asset)
		inline static Ref<Scene> GetRuntimeScene() { return s_ActiveProject->m_Config.RuntimeScene; }
		inline static const UUID& GetSceneHandle() { return s_ActiveProject->m_Config.SceneHandle; }
		
		inline static void Resize(const uint32_t& w, const uint32_t& h) { s_ActiveProject->m_Config.Resize(w, h); }

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
		* Example:
		*	output = projects/demo/assets/scripts/src
		*/
		inline static const std::filesystem::path GetScriptAssetPath()
		{
			GE_CORE_ASSERT(s_ActiveProject, "Cannot get Project Asset Path. No Active Project.");
			return GetPathToAsset(s_ActiveProject->m_Config.ScriptAssetPath);
		}

		/*
		* Returns filesystem path from project dir to given asset path.
		* Expected output = projects/demo/assets/path
		* @param path : example "scenes/demoExample.scene"
		*/
		inline static const std::filesystem::path GetPathToAsset(const std::filesystem::path& path)
		{
			GE_CORE_ASSERT(s_ActiveProject, "Cannot get Project Asset Path. No Active Project.");
			return std::filesystem::path(s_ActiveProject->m_Config.ProjectPath / s_ActiveProject->m_Config.AssetPath / path);
		}

		/*
		* Returns filesystem path from project dir to given script asset path
		* Expected output = projects/demo/assets/scripts/src/path
		* @param path = demo/Player.cs
		*/
		inline static const std::filesystem::path GetPathToScriptAsset(const std::filesystem::path& path)
		{
			GE_CORE_ASSERT(s_ActiveProject, "Cannot get Project Asset Path. No Active Project.");
			return std::filesystem::path(GetScriptAssetPath() / path);
		}

		/*
		* Example output = scripts/bin/GE-ScriptCore.dll
		*/
		inline static const std::filesystem::path& GetScriptCorePath() { return s_ActiveProject->m_Config.ScriptCorePath; }
		/*
		* Example output = scripts/bin/projName.dll
		*/
		inline static const std::filesystem::path& GetScriptAppPath() { return s_ActiveProject->m_Config.ScriptAppPath; }

#pragma region Asset Manager & Asset Control

		inline static Ref<AssetManager> GetAssetManager() { return s_ActiveProject->m_AssetManager; }

		/*
		* Types of Asset Managers
		*	1: Runtime, read from binary file
		*	2: Editor, read from text file
		*/
		template<typename T, typename ... Args>
		inline static Ref<T> NewAssetManager(Args&& ... args)
		{
			Ref<T> assetManager = CreateRef<T>(args...);
			s_ActiveProject->m_AssetManager = assetManager;
			return static_ref_cast<T, AssetManager>(s_ActiveProject->m_AssetManager);
		}

		template<typename T>
		inline static Ref<T> GetAssetManager()
		{
			return static_ref_cast<T, AssetManager>(s_ActiveProject->m_AssetManager);
		}

		template<typename T>
		inline static Ref<T> GetAssetAs(Ref<Asset> asset)
		{
			return static_ref_cast<T, Asset>(asset);
		}

		template<typename T>
		inline static Ref<T> GetAsset(UUID handle)
		{
			Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
			return GetAssetAs<T>(asset);
		}

		template<typename T>
		inline static Ref<T> GetCopyOfAsset(Ref<Asset> asset)
		{
			return GetAssetAs<T>(asset->GetCopy());
		}

		template<typename T>
		inline static Ref<T> GetAssetCopy(UUID handle)
		{
			Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
			return GetCopyOfAsset<T>(asset);
		}

#pragma endregion

		static void Init();
		static void Shutdown();

		static bool Load(const std::filesystem::path& path);
		static bool Save(const std::filesystem::path& path);

		/*
		* Stops current Scene if handle is set, then gets Scene Copy and sets new Scene handle
		*/
		static void SetSceneHandle(UUID handle);

		/*
		* 
		* @param state : Scene State enum. 0 = Stop, 1 = Run, 2 = Pause
		*/
		static void StartScene(const Scene::State& state) { s_ActiveProject->SceneStart(state); }
		static void UpdateScene(Timestep ts) { s_ActiveProject->SceneUpdate(ts); }
		static void StopScene() { s_ActiveProject->SceneStop(); }
		static void StepScene(int steps) { s_ActiveProject->SceneStep(steps); }
		static bool EventScene(Event& e, Entity entity);
		static Ref<Scene> ResetScene() { return s_ActiveProject->SceneReset(); }

		static const std::map<uint32_t, std::string>& GetTags() { return s_ActiveProject->m_Config.AllTags; }
		/*
		* Returns string value from AllTags map using id key
		* if id key is not found, will initialize and return id = 0 to "Default"
		*/
		static const std::string& GetStrByTag(uint32_t id);
		/*
		* Searches Config::AllTags for tag value, then returns id key. 
		* If not found, Returns 0.
		* @param tag : tag string. 
		*/
		static const uint32_t GetTagFromStr(const std::string& tag);
		static bool TagExists(const std::string& tag);
		static bool TagIDExists(uint32_t id);
		static bool AddTag(const std::string& tag, uint32_t id);
		static bool RemoveTag(uint32_t id);
	
	private:
		void SceneStart(const Scene::State& state);
		void SceneUpdate(Timestep ts);
		void SceneStop();
		void SceneStep(int steps);
		/*
		* If RuntimeScene exists, will call Scripting Events for provided Entity & return if handled. See: GE-ScriptCore::Entity.
		* Scene needs to be Running || Paused, and an EntityScriptFieldMap must exist for Entity.IDComponent.
		* @param e : Event to handle
		* @param entityID : internal entityID of Entity 
		*/
		bool SceneEvent(Event& e, Entity entity);
		/*
		* Creates temp/copy Scene using current SceneHandle and AssetManager
		* if successful, will Stop RuntimeScene if necessary amd set new RuntimeScene
		* * returns true
		* else
		* * returns false
		*/
		Ref<Scene> SceneReset();
	private: 
		Config m_Config = Config();
		Ref<AssetManager> m_AssetManager;

		static Ref<Project> s_ActiveProject;
	};

}