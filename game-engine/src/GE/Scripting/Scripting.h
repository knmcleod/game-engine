#pragma once

#include "GE/Scene/Scene.h"
#include "GE/Scene/Entity/Entity.h"

#include <mono/metadata/object.h>
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
	enum class ScriptFieldType
	{
		None = 0,
		Char,
		Int, UInt, Float, Byte, 
		Bool,
		Vector2, Vector3, Vector4,
		Entity
	};

	struct ScriptField
	{
		std::string Name;
		ScriptFieldType Type;

		MonoClassField* Field;
	};

	class ScriptFieldInstance
	{
	public:
		ScriptField Field;
	
		ScriptFieldInstance()
		{
			memset(m_ValueBuffer, 0, sizeof(m_ValueBuffer));
		}

		template<typename T>
		T GetValue()
		{
			return *(T*)m_ValueBuffer;
		}

		template<typename T>
		void SetValue(T value)
		{
			memcpy(m_ValueBuffer, &value, sizeof(T));
		}
	private:
		char m_ValueBuffer[8];

		friend class Scripting;
	};

	using ScriptFieldMap = std::map<std::string, ScriptFieldInstance>;
	
	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);

		MonoObject* Instantiate();
		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);

		const std::map<std::string, ScriptField>& GetFields() const { return m_Fields; }

	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		std::map<std::string, ScriptField> m_Fields;

		MonoClass* m_MonoClass = nullptr;

		friend class Scripting;
	};

	class ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, UUID uuid);

		void InvokeOnCreate();
		void InvokeOnUpdate(float timestep);

		Ref<ScriptClass> GetScriptClass() { return m_ScriptClass; }

		template<typename T>
		T GetFieldValue(const std::string& name)
		{
			Internal_GetFieldValue(name, s_FieldValueBuffer);
			return *(T*)s_FieldValueBuffer;
		}

		template<typename T>
		void SetFieldValue(const std::string& name, const T& data)
		{
			Internal_SetFieldValue(name, &data);
		}
	private:
		void Internal_GetFieldValue(const std::string& name, void* buffer);
		void Internal_SetFieldValue(const std::string& name, const void* value);
	private:
		Ref<ScriptClass> m_ScriptClass;
		MonoObject* m_Instance = nullptr;

		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreate = nullptr;
		MonoMethod* m_OnUpdate = nullptr;

		inline static char s_FieldValueBuffer[8];

		friend class Scripting;
	};

	class Scripting
	{
	public:
		static void SetScene(Scene* scene);
		static Scene* GetScene();

		static Ref<ScriptInstance> GetScriptInstance(UUID uuid);
		static Ref<ScriptClass> GetScriptClass(const std::string& name);

		static ScriptFieldMap& GetScriptFieldMap(Entity entity);

		// fullName includes namespace
		static bool ScriptClassExists(const std::string& fullName);

		static void Init();
		static void Shutdown();

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

		static char* ReadBytes(const std::filesystem::path& filePath, uint32_t* fileSize);

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