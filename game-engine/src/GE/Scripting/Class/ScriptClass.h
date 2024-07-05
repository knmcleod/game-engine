#pragma once

#include "../ScriptingUtils.h"

#include "GE/Core/UUID/UUID.h"

#include <map>
#include <string>

namespace GE
{
	class ScriptField
	{
		friend class Scripting;
		friend class ScriptClass;
		friend class ScriptInstance;
		friend class AssetSerializer;
	public:
		// TODO:: math wrapper
		enum class Type
		{
			None = 0,
			Char,
			Int, UInt, Float, Byte,
			Bool,
			Vector2, Vector3, Vector4,
			Entity
		};

		struct Data
		{
			std::string Name = "Script Field";
			Type FieldType = Type::None;

			MonoClassField* ClassField = nullptr;
		};

		ScriptField()
		{
			memset(m_ValueBuffer, 0, sizeof(m_ValueBuffer));
		}

		ScriptField(const std::string& name, Type type, MonoClassField* classField) : ScriptField()
		{
			m_Data.Name = name;
			m_Data.FieldType = type;
			m_Data.ClassField = classField;
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

		const Data& GetField() const { return m_Data; }
		const Type& GetType() const { return m_Data.FieldType; }
	private:
		char m_ValueBuffer[16];
		Data m_Data;

	};

	using ScriptFieldMap = std::map<std::string, ScriptField>;

	class ScriptClass
	{
		friend class Scripting;
	
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);

		MonoObject* Instantiate();
		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);

		// Example: "namespace.className"
		const std::string& GetFullName() const { return std::string(m_ClassNamespace + "." + m_ClassName); }
		const ScriptFieldMap& GetFields() const { return m_Fields; }

	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		ScriptFieldMap m_Fields;

		MonoClass* m_MonoClass = nullptr;

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
}