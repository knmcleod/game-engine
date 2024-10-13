#include "GE/GEpch.h"

#include "Scripting.h"

#include "GE/Asset/Assets/Scene/Scene.h"
#include "GE/Audio/AudioManager.h"

#include "GE/Core/Application/Application.h"
#include "GE/Core/FileSystem/FileSystem.h"
#include "GE/Core/Input/Input.h"

#include "GE/Physics/Physics.h"
#include "GE/Project/Project.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>
#pragma warning(pop)

#include <glm/glm.hpp>

namespace GE
{
#define GE_ADD_INTERNAL_CALL(Name) mono_add_internal_call("GE.InternalCalls::" #Name, Name)
	
#pragma region Internal Call Declarations

#pragma region Log Internal Calls

	static void Log_Core_Info(MonoString* debugMessage)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			std::string debug = Scripting::MonoStringToString(debugMessage);
			GE_CORE_INFO(debug);
		}
	}

	static void Log_Core_Trace(MonoString* debugMessage)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			std::string debug = Scripting::MonoStringToString(debugMessage);
			GE_CORE_TRACE(debug);
		}
	}

	static void Log_Core_Warn(MonoString* debugMessage)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			std::string debug = Scripting::MonoStringToString(debugMessage);
			GE_CORE_WARN(debug);
		}

	}

	static void Log_Core_Error(MonoString* debugMessage)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			std::string debug = Scripting::MonoStringToString(debugMessage);
			GE_CORE_ERROR(debug);
		}

	}

	static void Log_Core_Assert(MonoObject* object, MonoString* debugMessage)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			std::string debug = Scripting::MonoStringToString(debugMessage);
			GE_CORE_ASSERT(!object, debug);
		}
	}

#pragma endregion

#pragma region Entity & Component Internal Calls
	static bool Entity_HasComponent(UUID uuid, MonoReflectionType* componentType)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			if (Entity entity = scene->GetEntityByUUID(uuid))
			{
				MonoType* type = mono_reflection_type_get_type(componentType);
				GE_CORE_ASSERT(Scripting::HasComponentFunc(type), "Unable to find Component Type.");
				return Scripting::GetHasComponentFunc(type, entity);
			}
		}
		GE_CORE_ERROR("Scripting: Could not determine if Entity has Component.");
		return false;
	}

	static uint64_t Entity_FindEntityByName(MonoString* name)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			std::string nameStr = Scripting::MonoStringToString(name);
			if (Entity entity = scene->GetEntityByName(nameStr))
				return (uint64_t)entity.GetComponent<IDComponent>().ID;
		}
		return 0;
	}

	static MonoObject* Entity_GetScriptInstance(uint64_t uuid)
	{
		if(Ref<ScriptInstance> instance = Scripting::GetScriptInstance(uuid))
			return instance->GetMonoObject();
		return nullptr;
	}

// Transform Component
	static void TransformComponent_GetTranslation(UUID uuid, glm::vec3* translation)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<TransformComponent>())
			{
				auto& tc = entity.GetComponent<TransformComponent>();
				*translation = tc.Translation;
			}
		}
	}

	static void TransformComponent_SetTranslation(UUID uuid, glm::vec3* translation)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<TransformComponent>())
			{
				auto& tc = entity.GetComponent<TransformComponent>();
				tc.Translation = *translation;

				// Special case if Entity has Camera
				// Makes sure Cameras view matrix matches transform
				if (entity.HasComponent<CameraComponent>())
				{
					auto& cc = entity.GetComponent<CameraComponent>();
					cc.ActiveCamera.SetViewMatrix(tc.GetTransform());
					cc.ActiveCamera.SetPosition(tc.Translation);
				}
			}
		}
	}
	
// Audio Component
	static void AudioSourceComponent_GetLoop(UUID uuid, bool* loop)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<AudioSourceComponent>())
			{
				auto& asc = entity.GetComponent<AudioSourceComponent>();
				*loop = asc.Loop;
			}
		}
	}

	static void AudioSourceComponent_SetLoop(UUID uuid, bool* loop)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<AudioSourceComponent>())
			{
				auto& asc = entity.GetComponent<AudioSourceComponent>();
				asc.Loop = *loop;
			}
		}
	}

	static void AudioSourceComponent_GetPitch(UUID uuid, float* pitch)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<AudioSourceComponent>())
			{
				auto& asc = entity.GetComponent<AudioSourceComponent>();
				*pitch = asc.Pitch;
			}
		}
	}

	static void AudioSourceComponent_SetPitch(UUID uuid, float* pitch)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<AudioSourceComponent>())
			{
				auto& asc = entity.GetComponent<AudioSourceComponent>();
				asc.Pitch = *pitch;
			}
		}
	}

	static void AudioSourceComponent_GetGain(UUID uuid, float* gain)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<AudioSourceComponent>())
			{
				auto& asc = entity.GetComponent<AudioSourceComponent>();
				*gain = asc.Gain;
			}
		}
	}

	static void AudioSourceComponent_SetGain(UUID uuid, float* gain)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<AudioSourceComponent>())
			{
				auto& asc = entity.GetComponent<AudioSourceComponent>();
				asc.Gain = *gain;
			}
		}
	}

	static void AudioSourceComponent_Play(UUID uuid, glm::vec3* translation = nullptr, glm::vec3* velocity = nullptr)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<AudioSourceComponent>())
			{
				glm::vec3 position = glm::vec3(0.0);
				if (translation)
					position = *translation;
				else if (entity.HasComponent<TransformComponent>())
				{
					auto& trsc = entity.GetComponent<TransformComponent>();
					position = trsc.Translation;
				}

				glm::vec3 v = glm::vec3(0.0);
				if (velocity)
					v = *velocity;
				else if (entity.HasComponent<Rigidbody2DComponent>())
				{
					auto& rb2D = entity.GetComponent<Rigidbody2DComponent>();
					if (rb2D.RuntimeBody)
					{
						b2Body* body = (b2Body*)rb2D.RuntimeBody;
						const auto& lv = body->GetLinearVelocity();
						v = glm::vec3(lv.x, lv.y, 0.0);
					}
				}

				auto& asc = entity.GetComponent<AudioSourceComponent>();
				asc.Play(position, v);
			}
		}
	}

// Rigidbody2D Component
	// Point is World position
	static void Rigidbody2DComponent_ApplyLinearImpulse(UUID uuid, glm::vec2* impluse, glm::vec2* point, bool wake)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<Rigidbody2DComponent>())
			{
				auto& rbc = entity.GetComponent<Rigidbody2DComponent>();
				b2Body* body = (b2Body*)rbc.RuntimeBody;
				body->ApplyLinearImpulse(b2Vec2(impluse->x, impluse->y), b2Vec2(point->x, point->y), wake);

			}
		}
	}

	static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID uuid, glm::vec2* impluse, bool wake)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<Rigidbody2DComponent>())
			{
				auto& rbc = entity.GetComponent<Rigidbody2DComponent>();
				b2Body* body = (b2Body*)rbc.RuntimeBody;
				body->ApplyLinearImpulseToCenter(b2Vec2(impluse->x, impluse->y), wake);

			}
		}
	}

	static void Rigidbody2DComponent_GetLinearVelocity(UUID uuid, glm::vec2* velocity)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<Rigidbody2DComponent>())
			{
				auto& rbc = entity.GetComponent<Rigidbody2DComponent>();
				b2Body* body = (b2Body*)rbc.RuntimeBody;
				const b2Vec2& linearVelocity = body->GetLinearVelocity();
				*velocity = glm::vec2(linearVelocity.x, linearVelocity.y);
			}
		}
	}

	static Rigidbody2DComponent::BodyType Rigidbody2DComponent_GetType(UUID entityID)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(entityID);
			if (entity && entity.HasComponent<Rigidbody2DComponent>())
			{
				auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
				b2Body* body = (b2Body*)rb2d.RuntimeBody;
				return Physics::Rigidbody2DTypeFromBox2DBody(body->GetType());
			}
		}
		return Rigidbody2DComponent::BodyType::Static;
	}

	static void Rigidbody2DComponent_SetType(UUID entityID, Rigidbody2DComponent::BodyType bodyType)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(entityID);
			if (entity && entity.HasComponent<Rigidbody2DComponent>())
			{
				auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
				b2Body* body = (b2Body*)rb2d.RuntimeBody;
				body->SetType(Physics::Rigidbody2DTypeToBox2DBody(bodyType));
			}
		}
	}
	
// Text Renderer Component
	static MonoString* TextRendererComponent_GetText(UUID uuid)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<TextRendererComponent>())
			{
				auto& trc = entity.GetComponent<TextRendererComponent>();
				return Scripting::StringToMonoString(trc.Text.c_str());
			}
		}
		return nullptr;
	}

	static void TextRendererComponent_SetText(UUID uuid, MonoString* textString)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<TextRendererComponent>())
			{
				auto& trc = entity.GetComponent<TextRendererComponent>();
				trc.Text = Scripting::MonoStringToString(textString);
			}
		}
	}

	static void TextRendererComponent_GetTextColor(UUID uuid, glm::vec4* textColor)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<TextRendererComponent>())
			{
				auto& trc = entity.GetComponent<TextRendererComponent>();
				*textColor = trc.TextColor;
			}
		}
	}

	static void TextRendererComponent_SetTextColor(UUID uuid, glm::vec4* textColor)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<TextRendererComponent>())
			{
				auto& trc = entity.GetComponent<TextRendererComponent>();
				trc.TextColor = *textColor;
			}
		}
	}

	static void TextRendererComponent_GetBGColor(UUID uuid, glm::vec4* bgColor)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<TextRendererComponent>())
			{
				auto& trc = entity.GetComponent<TextRendererComponent>();
				*bgColor = trc.BGColor;
			}
		}
	}

	static void TextRendererComponent_SetBGColor(UUID uuid, glm::vec4* bgColor)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<TextRendererComponent>())
			{
				auto& trc = entity.GetComponent<TextRendererComponent>();
				trc.BGColor = *bgColor;
			}
		}
	}

	static float TextRendererComponent_GetLineHeight(UUID uuid)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<TextRendererComponent>())
			{
				auto& trc = entity.GetComponent<TextRendererComponent>();
				return trc.LineHeightOffset;
			}
		}
		return 0.0f;
	}

	static void TextRendererComponent_SetLineHeight(UUID uuid, float lineHeight)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<TextRendererComponent>())
			{
				auto& trc = entity.GetComponent<TextRendererComponent>();
				trc.LineHeightOffset = lineHeight;
			}
		}
	}

	static float TextRendererComponent_GetLineSpacing(UUID uuid)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<TextRendererComponent>())
			{
				auto& trc = entity.GetComponent<TextRendererComponent>();
				return trc.KerningOffset;
			}
		}
		return 0.0f;
	}

	static void TextRendererComponent_SetLineSpacing(UUID uuid, float lineSpacing)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && entity.HasComponent<TextRendererComponent>())
			{
				auto& trc = entity.GetComponent<TextRendererComponent>();
				trc.KerningOffset = lineSpacing;
			}
		}
	}

#pragma endregion

	static bool Input_IsKeyDown(Input::KeyCode keyCode)
	{
		return Input::IsKeyPressed(keyCode);
	}

	static bool Input_IsMouseDown(Input::MouseCode mouseCode)
	{
		return Input::IsMouseButtonPressed(mouseCode);
	}
#pragma endregion

#pragma region Scripting

	Scripting::Data Scripting::s_Data = Scripting::Data();

	std::unordered_map<MonoType*, std::function<bool(Entity)>> Scripting::s_HasComponentsFuncs = std::unordered_map<MonoType*, std::function<bool(Entity)>>();

	std::unordered_map<std::string, ScriptField::Type> Scripting::s_ScriptFieldTypeNames =
	{
		{ "System.Char",	ScriptField::Type::Char		},
		{ "System.Int",		ScriptField::Type::Int		},
		{ "System.UInt",	ScriptField::Type::UInt		},
		{ "System.Single",	ScriptField::Type::Float	},
		{ "System.Byte",	ScriptField::Type::Byte		},
		{ "System.Boolean",	ScriptField::Type::Bool		},
		{ "GE.Vector2",		ScriptField::Type::Vector2	},
		{ "GE.Vector3",		ScriptField::Type::Vector3	},
		{ "GE.Vector4",		ScriptField::Type::Vector4	},
		{ "GE.Entity",		ScriptField::Type::Entity	}
	};

#pragma region Helpers

	void Scripting::PrintMonoAssemblyTypes(MonoAssembly* assembly)
	{
		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionTable);

		GE_CORE_TRACE("Assembly Types");
		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			GE_CORE_TRACE("\n\tNamespace : {0}\n\tClassName: {1}", nameSpace, name);
		}
	}

	ScriptField::Type Scripting::MonoTypeToScriptFieldType(MonoType* monoType)
	{
		const char* typeName = mono_type_get_name(monoType);
		ScriptField::Type type = s_ScriptFieldTypeNames.at(typeName);

		if (s_ScriptFieldTypeNames.find(typeName) == s_ScriptFieldTypeNames.end())
			return ScriptField::Type::None;

		return s_ScriptFieldTypeNames.at(typeName);
	}

	MonoString* Scripting::StringToMonoString(const char* string)
	{
		return mono_string_new(s_Data.AppDomain, string);
	}

	std::string Scripting::MonoStringToString(MonoString* string)
	{
		char* cStr = mono_string_to_utf8(string);
		std::string str(cStr);
		mono_free(cStr);
		return str;
	}

	ScriptField::Type Scripting::StringToScriptFieldType(const std::string& typeStr)
	{
		if (typeStr == "None")		return ScriptField::Type::None;
		if (typeStr == "Char")		return ScriptField::Type::Char;
		if (typeStr == "Int")		return ScriptField::Type::Int;
		if (typeStr == "UInt")		return ScriptField::Type::UInt;
		if (typeStr == "Float")		return ScriptField::Type::Float;
		if (typeStr == "Byte")		return ScriptField::Type::Byte;
		if (typeStr == "Bool")		return ScriptField::Type::Bool;
		if (typeStr == "Vector2")	return ScriptField::Type::Vector3;
		if (typeStr == "Vector3")	return ScriptField::Type::Vector3;
		if (typeStr == "Vector4")	return ScriptField::Type::Vector4;
		if (typeStr == "Entity")	return ScriptField::Type::Entity;

		GE_CORE_ASSERT(false, "Unknown ScriptField Type.")
			return ScriptField::Type::None;
	}

	const char* Scripting::ScriptFieldTypeToString(ScriptField::Type fieldType)
	{
		switch (fieldType)
		{
		case GE::ScriptField::Type::None:
			return "None";
			break;
		case GE::ScriptField::Type::Char:
			return "Char";
			break;
		case GE::ScriptField::Type::Int:
			return "Int";
			break;
		case GE::ScriptField::Type::UInt:
			return "UInt";
			break;
		case GE::ScriptField::Type::Float:
			return "Float";
			break;
		case GE::ScriptField::Type::Byte:
			return "Byte";
			break;
		case GE::ScriptField::Type::Bool:
			return "Boolean";
			break;
		case GE::ScriptField::Type::Vector2:
			return "Vector2";
			break;
		case GE::ScriptField::Type::Vector3:
			return "Vector3";
			break;
		case GE::ScriptField::Type::Vector4:
			return "Vector4";
			break;
		case GE::ScriptField::Type::Entity:
			return "Entity";
			break;
		}
		return "<Invalid>";
	}
#pragma endregion

	MonoMethod* Scripting::GetEntityConstructor()
	{
		return Scripting::GetMethod(s_Data.EntityScript, ".ctor", 1);
	}

	Ref<ScriptInstance> Scripting::GetScriptInstance(UUID uuid)
	{
		if (s_Data.EntityScriptInstances.find(uuid) != s_Data.EntityScriptInstances.end())
			return s_Data.EntityScriptInstances.at(uuid);
		GE_CORE_WARN("Cannot find Script Instance by UUID. Returning nullptr.");
		return nullptr;
	}

	ScriptFieldMap& Scripting::GetEntityFields(UUID uuid)
	{
		return s_Data.EntityScriptFields[uuid];
	}

	Ref<Script> Scripting::GetScript(const std::string& fullName)
	{
		if (s_Data.Scripts.find(fullName) != s_Data.Scripts.end())
			return s_Data.Scripts.at(fullName);
		GE_CORE_WARN("Cannot find Script by fullName. Returning nullptr.");
		return nullptr;
	}

	Ref<Script> Scripting::GetScript(UUID handle)
	{
		for (const auto& [fullName, script] : s_Data.Scripts)
		{
			if (script->GetHandle() == handle)
				return script;
		}
		GE_CORE_WARN("Cannot find Script by uuid. Returning nullptr.");
		return nullptr;
	}

	void Scripting::SetScriptHandle(const std::string& fullName, UUID handle)
	{
		if (handle && !fullName.empty() && s_Data.Scripts.find(fullName) != s_Data.Scripts.end())
			s_Data.Scripts.at(fullName)->p_Handle = handle;
	}

	MonoMethod* Scripting::GetMethod(UUID scriptHandle, const std::string& name, int parameterCount)
	{
		if (Ref<Script> script = GetScript(scriptHandle))
			return GetMethod(script, name, parameterCount);
		return nullptr;
	}

	MonoMethod* Scripting::GetMethod(Ref<Script> script, const std::string& name, int parameterCount)
	{
		return mono_class_get_method_from_name(script->m_Class, name.c_str(), parameterCount);
	}

	MonoObject* Scripting::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		MonoObject* exception = nullptr;
		mono_thread_attach(s_Data.AppDomain);
		return mono_runtime_invoke(method, instance, params, &exception);
	}

	bool Scripting::ScriptExists(const std::string& fullName)
	{
		return s_Data.Scripts.find(fullName) != s_Data.Scripts.end();
	}

	bool Scripting::ScriptExists(UUID handle)
	{
		for (const auto& [fullname, script] : s_Data.Scripts)
		{
			if (script->GetHandle() == handle)
				return true;
		}
		return false;
	}

	bool Scripting::HasComponentFunc(MonoType* type)
	{
		return s_HasComponentsFuncs.find(type) != s_HasComponentsFuncs.end();
	}

	bool Scripting::GetHasComponentFunc(MonoType* type, Entity entity)
	{
		return s_HasComponentsFuncs.at(type)(entity);
	}

	MonoObject* Scripting::InstantiateScriptObject(UUID scriptHandle, bool isCore)
	{
		MonoObject* instance = nullptr;
		if (ScriptExists(scriptHandle))
		{
			Ref<Script> script = GetScript(scriptHandle);
			MonoDomain* domain = (isCore ? s_Data.RootDomain : s_Data.AppDomain);
			instance = mono_object_new(domain, script->m_Class);
			mono_runtime_object_init(instance);
		}
		return instance;
	}

	void Scripting::InstantiateScriptClass(Ref<Script> script, bool isCore)
	{
		MonoImage* monoImage = isCore ? GetCoreImage() : GetAppImage();
		script->m_Class = mono_class_from_name(monoImage, script->m_ClassNamespace.c_str(), script->m_ClassName.c_str());
	}

	void Scripting::Init()
	{
		GE_CORE_INFO("Scripting Init Start");

		InitMono();
		RegisterFunctions();

		LoadCoreAssembly(Project::GetPathToAsset(Project::GetScriptCorePath()));
		LoadAppAssembly(Project::GetPathToAsset(Project::GetScriptAppPath()));

		LoadAssemblyClasses();

		RegisterComponents();

		s_Data.InstantiateBaseEntity();
		GE_CORE_INFO("Scripting Init Complete");
	}

	void Scripting::Shutdown()
	{
		GE_CORE_INFO("Scripting Shutdown Start");
		OnStop();

		ShutdownMono();

		s_Data.Scripts.clear();
		s_Data.Scripts = std::unordered_map<std::string, Ref<Script>>();

		for (auto& [uuid, instanceFieldMap] : s_Data.EntityScriptFields)
		{
			for (auto& [name, instanceField] : instanceFieldMap)
			{
				instanceField.m_DataBuffer.Release();
			}
		}
		s_Data.EntityScriptFields.clear();
		s_Data.EntityScriptFields = std::unordered_map<UUID, ScriptFieldMap>();
		GE_CORE_INFO("Scripting Shutdown Complete");
	}

	void Scripting::ReloadAssembly()
	{
		GE_CORE_TRACE("Assembly Reload Started.");
		s_Data.AppAssemblyFileWatcher.reset();

		mono_domain_set(mono_get_root_domain(), false); // Set domain to root domain

		mono_domain_unload(s_Data.AppDomain); // Unload old app domain

		// Reload assemblies
		LoadCoreAssembly(Project::GetPathToAsset(Project::GetScriptCorePath()));
		LoadAppAssembly(Project::GetPathToAsset(Project::GetScriptAppPath()));

		LoadAssemblyClasses();

		RegisterComponents();

		s_Data.InstantiateBaseEntity();
		GE_CORE_TRACE("Assembly Reload Complete");
	}

	void Scripting::OnStop()
	{
		if (!s_Data.EntityScriptInstances.empty())
			s_Data.EntityScriptInstances.clear();
	}

	bool Scripting::OnEvent(Event& e, Entity entity)
	{
		UUID uuid = entity.GetComponent<IDComponent>().ID;
		if (!uuid || s_Data.EntityScriptInstances.find(uuid) == s_Data.EntityScriptInstances.end())
		{
			GE_CORE_WARN("Entity Script Instance that does not exist.");
			return false;
		}

		return s_Data.EntityScriptInstances.at(uuid)->InvokeOnEvent(e);
	}

	void Scripting::OnCreateScript(Entity entity)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();
		if (ScriptExists(sc.AssetHandle))
		{
			UUID uuid = entity.GetComponent<IDComponent>().ID;
			Ref<Script> script = GetScript(sc.AssetHandle);
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(sc.AssetHandle, uuid);
			s_Data.EntityScriptInstances.emplace(uuid, instance);

			// Script Fields
			{
				GE_PROFILE_SCOPE("Scripting::OnCreateScript(Entity) - Script Fields");
				bool entityFieldsExist = s_Data.EntityScriptFields.find(uuid) != s_Data.EntityScriptFields.end();
				const ScriptFieldMap& fields = (entityFieldsExist ? s_Data.EntityScriptFields.at(uuid) : script->m_Fields);

				for (const auto& [name, field] : fields)
				{
					switch (field.GetType())
					{
					case ScriptField::Type::Char:
					{
						const char& data = field.GetValue<char>();
						instance->SetFieldValue(name, data);
					}
						break;
					case ScriptField::Type::Int:
					{
						const int& data = field.GetValue<int>();
						instance->SetFieldValue(name, data);
					}
						break;
					case ScriptField::Type::UInt:
					{
						const uint32_t& data = field.GetValue<uint32_t>();
						instance->SetFieldValue(name, data);
					}
						break;
					case ScriptField::Type::Float:
					{
						const float& data = field.GetValue<float>();
						instance->SetFieldValue(name, data);
					}
						break;
					case ScriptField::Type::Byte:
					{
						const uint8_t& data = field.GetValue<uint8_t>();
						instance->SetFieldValue(name, data);
					}
						break;
					case ScriptField::Type::Bool:
					{
						const bool& data = field.GetValue<bool>();
						instance->SetFieldValue(name, data);
					}
						break;
					case ScriptField::Type::Vector2:
					{
						const glm::vec2& data = field.GetValue<glm::vec2>();
						instance->SetFieldValue(name, data);
					}
						break;
					case ScriptField::Type::Vector3:
					{
						const glm::vec3& data = field.GetValue<glm::vec3>();
						instance->SetFieldValue(name, data);
					}
						break;
					case ScriptField::Type::Vector4:
					{
						const glm::vec4& data = field.GetValue<glm::vec4>();
						instance->SetFieldValue(name, data);
					}
						break;
					case ScriptField::Type::Entity:
					{
						const UUID& data = field.GetValue<UUID>();
						instance->SetFieldValue(name, data);
					}
						break;
					}
				}

			}
			instance->InvokeOnCreate();
		}
	}

	void Scripting::OnUpdateScript(Entity entity, float timestep)
	{
		UUID uuid = entity.GetComponent<IDComponent>().ID;
		if (!uuid || s_Data.EntityScriptInstances.find(uuid) == s_Data.EntityScriptInstances.end())
		{
			GE_CORE_WARN("Tried to Update Entity/Script Instance that does not exist");
			return;
		}

		s_Data.EntityScriptInstances.at(uuid)->InvokeOnUpdate(timestep);
	}

	void Scripting::InitMono()
	{
		if (s_Data.RootDomain)
			return;

		GE_CORE_INFO("Mono Init Start");
		mono_set_assemblies_path("mono/lib");

		s_Data.RootDomain = mono_jit_init("GEJITRuntime");
		GE_CORE_ASSERT(s_Data.RootDomain, "Mono Scripting initialization failure.");

		mono_thread_set_main(mono_thread_current());

		GE_CORE_INFO("Mono Init Complete");
	}

	void Scripting::ShutdownMono()
	{
		GE_CORE_INFO("Mono Shutdown Start");
		if (s_Data.AppDomain && s_Data.RootDomain)
		{
			mono_domain_set(mono_get_root_domain(), false);
			mono_domain_unload(s_Data.AppDomain);
			s_Data.AppDomain = nullptr;

			mono_jit_cleanup(s_Data.RootDomain);
			s_Data.RootDomain = nullptr;
		}
		GE_CORE_INFO("Mono Shutdown Complete");

	}

	MonoAssembly* Scripting::LoadMonoAssembly(const std::filesystem::path& assemblyPath)
	{
		std::ifstream stream(assemblyPath, std::ios::binary | std::ios::ate);
		if (!stream)
		{
			GE_CORE_ASSERT(false, "Failed to open scripting file.");
			return nullptr;
		}

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint64_t size = end - stream.tellg();
		if (size == 0)
		{
			GE_CORE_ASSERT(false, "File is empty.");
			return nullptr;
		}

		char* data = new char[size];
		stream.read(data, size);
		stream.close();

		MonoImageOpenStatus monoStatus;
		MonoImage* image = mono_image_open_from_data_full(data,
			(uint32_t)size, 1, &monoStatus, 0);
		delete[] data;

		if (monoStatus != MONO_IMAGE_OK)
		{
			const char* errorMessage = mono_image_strerror(monoStatus);
			GE_CORE_ASSERT(false, errorMessage);
			return nullptr;
		}

		std::string pathString = assemblyPath.string();
		MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &monoStatus, 0);

		mono_image_close(image);
		return assembly;
	}

	void Scripting::LoadAppAssembly(const std::filesystem::path& filePath)
	{
		s_Data.AppAssembly = LoadMonoAssembly(filePath);
		s_Data.AppAssemblyImage = mono_assembly_get_image(s_Data.AppAssembly);
		//PrintMonoAssemblyTypes(s_Data.AppAssembly);

		// FileWatch setup - watches given app assembly filePath and call event
		s_Data.AppAssemblyFileWatcher =
				CreateScope<filewatch::FileWatch<std::string>>(filePath.string(), OnFileSystemAppAssemblyEvent);
		s_Data.AssemblyReloadPending = false;
	}

	void Scripting::LoadCoreAssembly(const std::filesystem::path& filePath)
	{
		// Create an App Domain
		s_Data.AppDomain = mono_domain_create_appdomain("GEScriptRuntime", nullptr);
		mono_domain_set(s_Data.AppDomain, true);

		s_Data.CoreAssembly = LoadMonoAssembly(filePath);
		s_Data.CoreAssemblyImage = mono_assembly_get_image(s_Data.CoreAssembly);
		//PrintMonoAssemblyTypes(s_Data.CoreAssembly);
	}
	
	void Scripting::LoadAssemblyClasses()
	{
		const MonoTableInfo* typeDefinitionTable = mono_image_get_table_info(s_Data.AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionTable);
		MonoClass* entityClass = mono_class_from_name(s_Data.CoreAssemblyImage, "GE", "Entity");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(s_Data.AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(s_Data.AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, name);
			else
				fullName = name;

			MonoClass* monoClass = mono_class_from_name(s_Data.AppAssemblyImage, nameSpace, name);
			bool isSubClass = mono_class_is_subclass_of(monoClass, entityClass, false);
			if (!isSubClass)
				continue;

			Ref<Script> script = CreateRef<Script>(nameSpace, name);
			s_Data.Scripts[fullName] = script;
			InstantiateScriptClass(script);

			void* iterator = nullptr;
			while (MonoClassField* monoField = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(monoField);

				uint32_t flags = mono_field_get_flags(monoField);
				if (flags & MONO_FIELD_ATTR_PUBLIC)
				{
					MonoType* monoType = mono_field_get_type(monoField);

					ScriptField::Type fieldType = MonoTypeToScriptFieldType(monoType);
					const char* fieldTypeName = ScriptFieldTypeToString(fieldType);

					// Clear any old data first
					if (script->m_Fields.find(fieldName) != script->m_Fields.end())
						script->m_Fields.at(fieldName).m_DataBuffer.Release();
					
					script->m_Fields[fieldName] = ScriptField(fieldTypeName, fieldType, monoField);
				}
			}
		}

	}

	void Scripting::RegisterFunctions()
	{
		GE_ADD_INTERNAL_CALL(Log_Core_Info);
		GE_ADD_INTERNAL_CALL(Log_Core_Trace);
		GE_ADD_INTERNAL_CALL(Log_Core_Warn);
		GE_ADD_INTERNAL_CALL(Log_Core_Error);
		GE_ADD_INTERNAL_CALL(Log_Core_Assert);

		GE_ADD_INTERNAL_CALL(Entity_HasComponent);
		GE_ADD_INTERNAL_CALL(Entity_FindEntityByName);
		GE_ADD_INTERNAL_CALL(Entity_GetScriptInstance);

		GE_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		GE_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);

		GE_ADD_INTERNAL_CALL(AudioSourceComponent_GetLoop);
		GE_ADD_INTERNAL_CALL(AudioSourceComponent_SetLoop);
		GE_ADD_INTERNAL_CALL(AudioSourceComponent_GetPitch);
		GE_ADD_INTERNAL_CALL(AudioSourceComponent_SetPitch);
		GE_ADD_INTERNAL_CALL(AudioSourceComponent_GetGain);
		GE_ADD_INTERNAL_CALL(AudioSourceComponent_SetGain);
		GE_ADD_INTERNAL_CALL(AudioSourceComponent_Play);

		GE_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		GE_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
		GE_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetLinearVelocity);
		GE_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetType);
		GE_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetType);

		GE_ADD_INTERNAL_CALL(TextRendererComponent_GetText);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_SetText);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_GetTextColor);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_SetTextColor);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_GetBGColor);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_SetBGColor);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_GetLineHeight);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_SetLineHeight);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_GetLineSpacing);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_SetLineSpacing);

		GE_ADD_INTERNAL_CALL(Input_IsKeyDown);
		GE_ADD_INTERNAL_CALL(Input_IsMouseDown);
	}

	template<typename T>
	void Scripting::RegisterComponent()
	{
		std::string typeName = typeid(T).name();
		size_t namePosition = typeName.find_last_of(':');
		std::string name = typeName.substr(namePosition + 1);
		std::string managedTypeName = fmt::format("GE.{}", name); // Namespace.ComponentName

		MonoType* managedType = mono_reflection_type_from_name(managedTypeName.data(), Scripting::s_Data.CoreAssemblyImage);
		if (!managedType) { GE_CORE_ERROR("Could not Register Component. Component Type not found."); return; }
		s_HasComponentsFuncs[managedType] = [](Entity entity) { return entity.HasComponent<T>(); };
	}

	void Scripting::RegisterComponents()
	{
		s_HasComponentsFuncs.clear();

		//RegisterComponent<IDComponent>();
		//RegisterComponent<TagComponent>();
		//RegisterComponent<NameComponent>();
		RegisterComponent<TransformComponent>();
		RegisterComponent<AudioSourceComponent>();
		//RegisterComponent<AudioListenerComponent>();
		//RegisterComponent<RenderComponent>();
		//RegisterComponent<CameraComponent>();
		//RegisterComponent<SpriteRendererComponent>();
		//RegisterComponent<CircleRendererComponent>();
		RegisterComponent<TextRendererComponent>();
		RegisterComponent<Rigidbody2DComponent>();
		//RegisterComponent<BoxCollider2DComponent>();
		//RegisterComponent<CircleCollider2DComponent>();
	}

	void Scripting::OnFileSystemAppAssemblyEvent(const std::string& path, const filewatch::Event changeType)
	{
		if (!s_Data.AssemblyReloadPending && changeType == filewatch::Event::modified)
		{
			s_Data.AssemblyReloadPending = true;

			Application::SubmitToMainAppThread([]()
				{	
					ReloadAssembly();
				});
		}
	}
#pragma endregion

#pragma region ScriptInstance
	ScriptInstance::ScriptInstance(UUID scriptHandle, UUID entityID) : m_ScriptHandle(scriptHandle)
	{
		m_Instance = Scripting::InstantiateScriptObject(scriptHandle);

		m_Constructor = Scripting::GetEntityConstructor();
		m_OnCreate = Scripting::GetMethod(scriptHandle, "OnCreate", 0);
		m_OnUpdate = Scripting::GetMethod(scriptHandle, "OnUpdate", 1);
		m_OnEvent = Scripting::GetMethod(scriptHandle, "OnEvent", 1);

		// Invoke Constructor
		if(m_Instance)
		{
			void* param = &entityID;
			Scripting::InvokeMethod(m_Instance, m_Constructor, &param);
		}

	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (m_OnCreate == NULL || m_Instance == NULL)
			return;
		Scripting::InvokeMethod(m_Instance, m_OnCreate, nullptr);
	}

	void ScriptInstance::InvokeOnUpdate(float timestep)
	{
		if (!m_Instance || !m_OnUpdate)
			return;

		void* param = &timestep;
		Scripting::InvokeMethod(m_Instance, m_OnUpdate, &param);
	}

	bool ScriptInstance::InvokeOnEvent(Event& e)
	{
		if (!m_Instance || !m_OnEvent)
			return false;

		Event::Type type = e.GetEventType();
		void* param = &type;
		Scripting::InvokeMethod(m_Instance, m_OnEvent, &param);
		// TODO : Get return from method
		return false;
	}

	void ScriptInstance::Internal_GetFieldValue(const std::string& name, void* buffer)
	{
		if (Ref<Script> script = Scripting::GetScript(m_ScriptHandle))
		{
			const auto& fields = script->GetFields();
			if (fields.find(name) == fields.end())
			{
				GE_CORE_WARN("Cannot get Script Instance Field Value. \n\t{0}", name);
				return;
			}

			const ScriptField& field = fields.at(name);
			mono_field_get_value(m_Instance, field.GetData().ClassField, buffer);
		}
	}

	void ScriptInstance::Internal_SetFieldValue(const std::string& name, const void* value)
	{
		if (Ref<Script> script = Scripting::GetScript(m_ScriptHandle))
		{
			const auto& fields = script->GetFields();
			auto instance = fields.find(name);
			if (instance == fields.end())
			{
				GE_CORE_WARN("Cannot set Script Instance Field Value. {0}", name);
				return;
			}

			const ScriptField& field = instance->second;
			mono_field_set_value(m_Instance, field.GetData().ClassField, (void*)value);
		}
	}
#pragma endregion

}