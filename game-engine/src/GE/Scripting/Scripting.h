#pragma once

#include "GE/Core/Buffer.h"
#include "GE/Scene/Scene.h"
#include "GE/Scene/Entity/Entity.h"

#include <filesystem>
#include <string>

#include <FileWatch.h>

extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoClassField MonoClassField;
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
		std::string Name = "Script Field";
		ScriptFieldType Type = ScriptFieldType::None;

		MonoClassField* Field = nullptr;
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
		char m_ValueBuffer[16];

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

		MonoObject* GetMonoObject() { return m_Instance; }
	private:
		void Internal_GetFieldValue(const std::string& name, void* buffer);
		void Internal_SetFieldValue(const std::string& name, const void* value);
	private:
		Ref<ScriptClass> m_ScriptClass;
		MonoObject* m_Instance = nullptr;

		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreate = nullptr;
		MonoMethod* m_OnUpdate = nullptr;

		inline static char s_FieldValueBuffer[16];

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

		static MonoObject* GetObjectInstance(UUID uuid);

		// fullName includes namespace
		static bool ScriptClassExists(const std::string& fullName);

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

	static ScriptFieldType StringToScriptFieldType(std::string_view type)
	{
		if (type == "None")		return ScriptFieldType::None;
		if (type == "Char")		return ScriptFieldType::Char;
		if (type == "Int")		return ScriptFieldType::Int;
		if (type == "UInt")		return ScriptFieldType::UInt;
		if (type == "Float")	return ScriptFieldType::Float;
		if (type == "Byte")		return ScriptFieldType::Byte;
		if (type == "Bool")		return ScriptFieldType::Bool;
		if (type == "Vector2")	return ScriptFieldType::Vector3;
		if (type == "Vector3")	return ScriptFieldType::Vector3;
		if (type == "Vector4")	return ScriptFieldType::Vector4;
		if (type == "Entity")	return ScriptFieldType::Entity;

		GE_CORE_ASSERT(false, "Unknown ScriptField Type.")
			return ScriptFieldType::None;
	}

	static const char* ScriptFieldTypeToString(ScriptFieldType fieldType)
	{
		switch (fieldType)
		{
		case GE::ScriptFieldType::None:
			return "None";
			break;
		case GE::ScriptFieldType::Char:
			return "Char";
			break;
		case GE::ScriptFieldType::Int:
			return "Int";
			break;
		case GE::ScriptFieldType::UInt:
			return "UInt";
			break;
		case GE::ScriptFieldType::Float:
			return "Float";
			break;
		case GE::ScriptFieldType::Byte:
			return "Byte";
			break;
		case GE::ScriptFieldType::Bool:
			return "Boolean";
			break;
		case GE::ScriptFieldType::Vector2:
			return "Vector2";
			break;
		case GE::ScriptFieldType::Vector3:
			return "Vector3";
			break;
		case GE::ScriptFieldType::Vector4:
			return "Vector4";
			break;
		case GE::ScriptFieldType::Entity:
			return "Entity";
			break;
		}
		return "<Invalid>";
	}
}