#pragma once

#include "GE/Scene/Scene.h"
#include "GE/Scene/Entity/Entity.h"

#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>

#include <filesystem>
#include <string>

extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
}

namespace GE
{
	class Scripting
	{
	public:
		static void Init();
		static void Shutdown();

		static void SetScene(Scene* scene);
		static Scene* GetScene();

		static void OnStop();

		// fullName includes namespace
		static bool ScriptClassExists(const std::string& fullName);

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

		static char* ReadBytes(const std::filesystem::path& filePath, uint32_t* fileSize);

		friend class ScriptClass;
		friend class ScriptGlue;
	};

	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);

		MonoObject* Instantiate();
		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);
	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		MonoClass* m_MonoClass = nullptr;
	};

	class ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, UUID uuid);

		void InvokeOnCreate();
		void InvokeOnUpdate(float timestep);

	private:
		Ref<ScriptClass> m_ScriptClass;
		MonoObject* m_Instance;

		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreate = nullptr;
		MonoMethod* m_OnUpdate = nullptr;
	};

	class ScriptGlue
	{
	public:
		static void RegisterFunctions();
		static void RegisterComponents();
	};
}