#pragma once

#include "ScriptingUtils.h"
#include "Class/ScriptClass.h"

#include "GE/Asset/Assets/Scene/Scene.h"

#include "GE/Core/Memory/Buffer.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <FileWatch.h>
#pragma warning(pop)

namespace GE
{
	class Scripting
	{
	public:
		struct Data
		{
			MonoDomain* RootDomain = nullptr;
			MonoDomain* AppDomain = nullptr;

			MonoAssembly* CoreAssembly = nullptr;
			MonoImage* CoreAssemblyImage = nullptr;
			MonoAssembly* AppAssembly = nullptr;
			MonoImage* AppAssemblyImage = nullptr;

			std::filesystem::path CoreAssemblyFilepath;
			std::filesystem::path AppAssemblyFilepath;

			Scope<filewatch::FileWatch<std::string>> AppAssemblyFileWatcher;
			bool AssemblyReloadPending = false;

			ScriptClass EntityClass;

			std::unordered_map<std::string, Ref<ScriptClass>> ScriptClasses;
			std::unordered_map<UUID, Ref<ScriptInstance>> ScriptInstances;

			std::unordered_map<UUID, ScriptFieldMap> ScriptFields;

			Scene* ActiveScene = nullptr;

		};

		static Data* s_Data;

		inline static void SetScene(const Scene* scene) { s_Data->ActiveScene = (Scene*)scene; }
		inline static Scene* GetScene() { return s_Data->ActiveScene; }

		static Ref<ScriptInstance> GetScriptInstance(UUID uuid);
		static Ref<ScriptClass> GetScriptClass(const std::string& name);

		static ScriptFieldMap& GetScriptFieldMap(Entity entity);

		static MonoObject* GetObjectInstance(UUID uuid);

		static void PrintMonoAssemblyTypes(MonoAssembly* assembly);

		static ScriptField::Type MonoTypeToScriptFieldType(MonoType* monoType);

		static MonoString* StringToMonoString(const char* string);
		static std::string MonoStringToString(MonoString* string);

		static ScriptField::Type StringToScriptFieldType(std::string_view type);
		static const char* ScriptFieldTypeToString(ScriptField::Type fieldType);

		// fullName includes namespace
		static bool ScriptClassExists(const std::string& fullName);

		/*
		* Called after Project initialization
		*/
		static void Init();
		static void Shutdown();

		static void ReloadAssembly();

		static void OnStop();
		static void OnCreateScript(Entity entity);
		static void OnUpdateScript(Entity entity, float timestep);
	private:
		static void InitMono();
		static void ShutdownMono();

		static MonoObject* InstantiateClass(MonoClass* monoClass);
		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath);
		static void LoadApplicationAssembly(const std::filesystem::path& filepath);
		static void LoadAssembly(const std::filesystem::path& filepath);
		static void LoadAssemblyClasses();

		static void OnApplicationAssemblyFileSystemEvent(const std::string& path, const filewatch::Event changeType);

		friend class ScriptClass;
		friend class ScriptGlue;
	};

	class ScriptGlue
	{
	public:
		static void RegisterFunctions();
		static void RegisterComponents();
	};

}