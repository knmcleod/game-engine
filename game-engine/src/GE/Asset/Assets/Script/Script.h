#pragma once

#include "GE/Asset/Assets/Asset.h"
#include "GE/Core/Memory/Buffer.h"
#include "GE/Scripting/ScriptingUtils.h"

#include <map>

namespace GE
{
	class ScriptField
	{
		friend class Script;
		friend class ScriptInstance;
		friend class Scripting;
	public:
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
	private:
		Buffer m_DataBuffer = Buffer();
		Data m_Data;
	public:
		ScriptField() = default;
		ScriptField(const std::string& name, Type type, MonoClassField* classField)
		{
			m_Data.Name = name;
			m_Data.FieldType = type;
			m_Data.ClassField = classField;
		}

		template<typename T>
		const T GetValue() const
		{
			if(m_DataBuffer)
				return *m_DataBuffer.As<T>();
			return T();
		}

		template<typename T>
		void SetValue(T& value)
		{
			m_DataBuffer.Release();
			m_DataBuffer = Buffer((void*)&value, sizeof(T));
		}

		const Data& GetData() const { return m_Data; }
		const Type& GetType() const { return m_Data.FieldType; }
		void SetType(Type type) { m_Data.FieldType = type; }
		void SetName(const std::string& name) { m_Data.Name = name; }

	};

	using ScriptFieldMap = std::map<std::string, ScriptField>;

	class Script : public Asset
	{
		friend class Scripting;
	public:
		Script();
		Script(const std::string& classNamespace, const std::string& className);
		~Script();

		/*
		* Returns class fullName. Example: "namespace.className"
		* Do not use for Scripting Asset filepath, 
		*	instead use Project::GetPathToScriptAsset("GetNamespace()/GetName()")
		* AssetMetadata filepath = "GetNamespace()/GetName()"
		*/
		const std::string GetFullName() const { return std::string(m_ClassNamespace + "." + m_ClassName); }
		
		const std::string& GetNamespace() const { return m_ClassNamespace; }
		const std::string& GetName() const { return m_ClassName; }
		/*
		* Returns map to default field values assigned in C# Script
		*/
		const ScriptFieldMap& GetFields() const { return m_Fields; }
		
		bool FieldExists(const std::string& name) 
		{
			return m_Fields.find(name) != m_Fields.end();
		}

	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		// Holds default field values assigned in C# Script
		ScriptFieldMap m_Fields;

		MonoClass* m_Class = nullptr;
	};
}
