#include "GE/GEpch.h"

#include "ScriptClass.h"
#include "../Scripting.h"

#include <mono/metadata/class.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/threads.h>

namespace GE
{
#pragma region ScriptClass

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(isCore ? Scripting::s_Data->CoreAssemblyImage : Scripting::s_Data->AppAssemblyImage, classNamespace.c_str(), className.c_str());
	}

	MonoObject* ScriptClass::Instantiate()
	{
		return Scripting::InstantiateClass(m_MonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{
		return mono_class_get_method_from_name(m_MonoClass, name.c_str(), parameterCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		MonoObject* exception = nullptr;
		mono_thread_attach(mono_get_root_domain());
		return mono_runtime_invoke(method, instance, params, &exception);
	}
#pragma endregion

#pragma region ScriptInstance
	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, UUID uuid) : m_ScriptClass(scriptClass)
	{
		m_Instance = m_ScriptClass->Instantiate();

		m_Constructor = Scripting::s_Data->EntityClass.GetMethod(".ctor", 1);
		m_OnCreate = m_ScriptClass->GetMethod("OnCreate", 0);
		m_OnUpdate = m_ScriptClass->GetMethod("OnUpdate", 1);

		// Invoke Constructor
		{
			void* param = &uuid;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
		}
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (!m_OnCreate || m_Instance == NULL || !m_ScriptClass)
			return;
		m_ScriptClass->InvokeMethod(m_Instance, m_OnCreate);
	}

	void ScriptInstance::InvokeOnUpdate(float timestep)
	{
		if (!m_Instance || !m_OnUpdate)
			return;

		void* param = &timestep;
		m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdate, &param);
	}

	void ScriptInstance::Internal_GetFieldValue(const std::string& name, void* buffer)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto instance = fields.find(name);
		if (instance == fields.end())
		{
			GE_CORE_WARN("Cannot get Script Instance Field Value. \n\t{0}", name);
			return;
		}

		const ScriptField& field = instance->second;
		mono_field_get_value(m_Instance, field.m_Data.ClassField, buffer);
	}

	void ScriptInstance::Internal_SetFieldValue(const std::string& name, const void* value)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto instance = fields.find(name);
		if (instance == fields.end())
		{
			GE_CORE_WARN("Cannot set Script Instance Field Value. {0}", name);
			return;
		}

		const ScriptField& field = instance->second;
		mono_field_set_value(m_Instance, field.m_Data.ClassField, (void*)value);
	}
#pragma endregion
}