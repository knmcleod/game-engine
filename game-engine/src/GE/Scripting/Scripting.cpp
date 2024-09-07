#include "GE/GEpch.h"

#include "Scripting.h"

#include "GE/Audio/AudioManager.h"

#include "GE/Core/Application/Application.h"
#include "GE/Core/FileSystem/FileSystem.h"
#include "GE/Core/Input/Input.h"

#include "GE/Physics/Physics.h"
#include "GE/Project/Project.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>

#include <glm/glm.hpp>

namespace GE
{
	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_HasComponentsFuncs;

	static std::unordered_map<std::string, ScriptField::Type>  s_ScriptFieldTypeNames
	{
		{ "System.Char",	ScriptField::Type::Char		},
		{ "System.Int",		ScriptField::Type::Int		},
		{ "System.UInt",	ScriptField::Type::UInt		},
		{ "System.Single",	ScriptField::Type::Float		},
		{ "System.Byte",	ScriptField::Type::Byte		},
		{ "System.Boolean",	ScriptField::Type::Bool		},
		{ "GE.Vector2",		ScriptField::Type::Vector2	},
		{ "GE.Vector3",		ScriptField::Type::Vector3	},
		{ "GE.Vector4",		ScriptField::Type::Vector4	},
		{ "GE.Entity",		ScriptField::Type::Entity		}
	};

#define GE_ADD_INTERNAL_CALL(Name) mono_add_internal_call("GE.InternalCalls::" #Name, Name)

#pragma region Internal Call Declarations

#pragma region Log Internal Calls

	static void Log_Core_Info(MonoString* debugMessage)
	{
		Scene* scene = Scripting::GetScene();
		GE_CORE_ASSERT(scene, "Scene is Undefined.");

		std::string debug = Scripting::MonoStringToString(debugMessage);
		GE_CORE_INFO(debug);
	}

	static void Log_Core_Trace(MonoString* debugMessage)
	{
		Scene* scene = Scripting::GetScene();
		GE_CORE_ASSERT(scene, "Scene is Undefined.");

		std::string debug = Scripting::MonoStringToString(debugMessage);
		GE_CORE_TRACE(debug);
	}

	static void Log_Core_Warn(MonoString* debugMessage)
	{
		Scene* scene = Scripting::GetScene();
		GE_CORE_ASSERT(scene, "Scene is Undefined.");

		std::string debug = Scripting::MonoStringToString(debugMessage);
		GE_CORE_WARN(debug);

	}

	static void Log_Core_Error(MonoString* debugMessage)
	{
		Scene* scene = Scripting::GetScene();
		GE_CORE_ASSERT(scene, "Scene is Undefined.");

		std::string debug = Scripting::MonoStringToString(debugMessage);
		GE_CORE_ERROR(debug);

	}

	static void Log_Core_Assert(MonoObject* object, MonoString* debugMessage)
	{
		Scene* scene = Scripting::GetScene();
		GE_CORE_ASSERT(scene, "Scene is Undefined.");

		std::string debug = Scripting::MonoStringToString(debugMessage);
		GE_CORE_ASSERT(!object, debug);
	}

#pragma endregion

#pragma region Entity & Component Internal Calls
	static bool Entity_HasComponent(UUID uuid, MonoReflectionType* componentType)
	{
		Scene* scene = Scripting::GetScene();
		GE_CORE_ASSERT(scene, "Scene is Undefined.");
		Entity entity = scene->GetEntityByUUID(uuid);
		GE_CORE_ASSERT(entity, "Entity is Undefined.");

		MonoType* type = mono_reflection_type_get_type(componentType);
		GE_CORE_ASSERT(s_HasComponentsFuncs.find(type) != s_HasComponentsFuncs.end(), "Unable to find Component Type.");
		return s_HasComponentsFuncs.at(type)(entity);
	}

	static uint64_t Entity_FindEntityByTag(MonoString* tagString)
	{
		Scene* scene = Scripting::GetScene();
		GE_CORE_ASSERT(scene, "Scene is Undefined.");

		std::string tag = Scripting::MonoStringToString(tagString);
		Entity entity = scene->GetEntityByTag(tag);

		if (!entity)
			return 0;

		return (uint64_t)entity.GetComponent<IDComponent>().ID;
	}

	static MonoObject* Entity_GetScriptInstance(uint64_t uuid)
	{
		return Scripting::GetObjectInstance(uuid);
	}

// Transform Component
	static void TransformComponent_GetTranslation(UUID uuid, glm::vec3* translation)
	{
		Scene* scene = Scripting::GetScene();
		
		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity && entity.HasComponent<TransformComponent>())
		{
			auto& tc = entity.GetComponent<TransformComponent>();
			*translation = tc.Translation;
		}
	}

	static void TransformComponent_SetTranslation(UUID uuid, glm::vec3* translation)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity.HasComponent<TransformComponent>())
		{
			auto& tc = entity.GetComponent<TransformComponent>();
			tc.Translation = *translation;
		}
	}
	
// Audio Component
	static void AudioSourceComponent_GetLoop(UUID uuid, bool* loop)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity && entity.HasComponent<AudioSourceComponent>())
		{
			auto& asc = entity.GetComponent<AudioSourceComponent>();
			*loop = asc.Loop;
		}
	}

	static void AudioSourceComponent_SetLoop(UUID uuid, bool* loop)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity.HasComponent<AudioSourceComponent>())
		{
			auto& asc = entity.GetComponent<AudioSourceComponent>();
			asc.Loop = *loop;
		}
	}

	static void AudioSourceComponent_GetPitch(UUID uuid, float* pitch)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity && entity.HasComponent<AudioSourceComponent>())
		{
			auto& asc = entity.GetComponent<AudioSourceComponent>();
			*pitch = asc.Pitch;
		}
	}

	static void AudioSourceComponent_SetPitch(UUID uuid, float* pitch)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity.HasComponent<AudioSourceComponent>())
		{
			auto& asc = entity.GetComponent<AudioSourceComponent>();
			asc.Pitch = *pitch;
		}
	}

	static void AudioSourceComponent_GetGain(UUID uuid, float* gain)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity && entity.HasComponent<AudioSourceComponent>())
		{
			auto& asc = entity.GetComponent<AudioSourceComponent>();
			*gain = asc.Gain;
		}
	}

	static void AudioSourceComponent_SetGain(UUID uuid, float* gain)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity.HasComponent<AudioSourceComponent>())
		{
			auto& asc = entity.GetComponent<AudioSourceComponent>();
			asc.Gain = *gain;
		}
	}

	static void AudioSourceComponent_Play(UUID uuid, glm::vec3* translation = nullptr, glm::vec3* velocity = nullptr)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity.HasComponent<AudioSourceComponent>())
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

// Rigidbody2D Component
	// Point is World position
	static void Rigidbody2DComponent_ApplyLinearImpulse(UUID uuid, glm::vec2* impluse, glm::vec2* point, bool wake)
	{
		Scene* scene = Scripting::GetScene();
		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			auto& rbc = entity.GetComponent<Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rbc.RuntimeBody;
			body->ApplyLinearImpulse(b2Vec2(impluse->x, impluse->y), b2Vec2(point->x, point->y), wake);

		}
	}

	static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID uuid, glm::vec2* impluse, bool wake)
	{
		Scene* scene = Scripting::GetScene();
		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			auto& rbc = entity.GetComponent<Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rbc.RuntimeBody;
			body->ApplyLinearImpulseToCenter(b2Vec2(impluse->x, impluse->y), wake);

		}
	}

	static void Rigidbody2DComponent_GetLinearVelocity(UUID uuid, glm::vec2* velocity)
	{
		Scene* scene = Scripting::GetScene();
		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			auto& rbc = entity.GetComponent<Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rbc.RuntimeBody;
			const b2Vec2& linearVelocity = body->GetLinearVelocity();
			*velocity = glm::vec2(linearVelocity.x, linearVelocity.y);
		}
	}

	static Rigidbody2DComponent::BodyType Rigidbody2DComponent_GetType(UUID entityID)
	{
		Scene* scene = Scripting::GetScene();
		Entity entity = scene->GetEntityByUUID(entityID);

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			return Physics::Rigidbody2DTypeFromBox2DBody(body->GetType());
		}

		return Rigidbody2DComponent::BodyType::Static;
	}

	static void Rigidbody2DComponent_SetType(UUID entityID, Rigidbody2DComponent::BodyType bodyType)
	{
		Scene* scene = Scripting::GetScene();
		Entity entity = scene->GetEntityByUUID(entityID);

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			body->SetType(Physics::Rigidbody2DTypeToBox2DBody(bodyType));
		}
	}
	
// Text Renderer Component
	static MonoString* TextRendererComponent_GetText(UUID uuid)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity && entity.HasComponent<TextRendererComponent>())
		{
			auto& trc = entity.GetComponent<TextRendererComponent>();
			return Scripting::StringToMonoString(trc.Text.c_str());
		}
		return nullptr;
	}

	static void TextRendererComponent_SetText(UUID uuid, MonoString* textString)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity.HasComponent<TextRendererComponent>())
		{
			auto& trc = entity.GetComponent<TextRendererComponent>();
			trc.Text = Scripting::MonoStringToString(textString);
		}
	}

	static void TextRendererComponent_GetTextColor(UUID uuid, glm::vec4* textColor)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity && entity.HasComponent<TextRendererComponent>())
		{
			auto& trc = entity.GetComponent<TextRendererComponent>();
			*textColor = trc.TextColor;
		}
	}

	static void TextRendererComponent_SetTextColor(UUID uuid, glm::vec4* textColor)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity.HasComponent<TextRendererComponent>())
		{
			auto& trc = entity.GetComponent<TextRendererComponent>();
			trc.TextColor = *textColor;
		}
	}

	static void TextRendererComponent_GetBGColor(UUID uuid, glm::vec4* bgColor)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity && entity.HasComponent<TextRendererComponent>())
		{
			auto& trc = entity.GetComponent<TextRendererComponent>();
			*bgColor = trc.BGColor;
		}
	}

	static void TextRendererComponent_SetBGColor(UUID uuid, glm::vec4* bgColor)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity.HasComponent<TextRendererComponent>())
		{
			auto& trc = entity.GetComponent<TextRendererComponent>();
			trc.BGColor = *bgColor;
		}
	}

	static float TextRendererComponent_GetLineHeight(UUID uuid)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity && entity.HasComponent<TextRendererComponent>())
		{
			auto& trc = entity.GetComponent<TextRendererComponent>();
			return trc.LineHeightOffset;
		}
		return 0.0f;
	}

	static void TextRendererComponent_SetLineHeight(UUID uuid, float lineHeight)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity.HasComponent<TextRendererComponent>())
		{
			auto& trc = entity.GetComponent<TextRendererComponent>();
			trc.LineHeightOffset = lineHeight;
		}
	}

	static float TextRendererComponent_GetLineSpacing(UUID uuid)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity && entity.HasComponent<TextRendererComponent>())
		{
			auto& trc = entity.GetComponent<TextRendererComponent>();
			return trc.KerningOffset;
		}
		return 0.0f;
	}

	static void TextRendererComponent_SetLineSpacing(UUID uuid, float lineSpacing)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntityByUUID(uuid);
		if (entity.HasComponent<TextRendererComponent>())
		{
			auto& trc = entity.GetComponent<TextRendererComponent>();
			trc.KerningOffset = lineSpacing;
		}
	}

#pragma endregion

	static bool Input_IsKeyDown(Input::KeyCode keyCode)
	{
		return Input::IsKeyPressed(keyCode);
	}
#pragma endregion

#pragma region Scripting

	Scripting::Data* Scripting::s_Data = new Data();

	Ref<ScriptInstance> Scripting::GetScriptInstance(UUID uuid)
	{
		auto instance = s_Data->ScriptInstances.find(uuid);
		if (instance == s_Data->ScriptInstances.end())
		{
			GE_CORE_WARN("Cannot find Script Instance by UUID. Returning nullptr.");
			return nullptr;
		}
		return instance->second;
	}

	Ref<ScriptClass> Scripting::GetScriptClass(const std::string& name)
	{
		auto scriptClass = s_Data->ScriptClasses.find(name);
		if (scriptClass == s_Data->ScriptClasses.end())
		{
			GE_CORE_WARN("Cannot find Script Class by name. Returning nullptr.");
			return nullptr;
		}
		return scriptClass->second;
	}

	ScriptFieldMap& Scripting::GetScriptFieldMap(Entity entity)
	{
		GE_CORE_ASSERT(entity, "Cannot get Entity ScriptFields. Entity does not exist.");

		UUID uuid = entity.GetComponent<IDComponent>().ID;

		return s_Data->ScriptFields[uuid];
	}

	MonoObject* Scripting::GetObjectInstance(UUID uuid)
	{
		if (s_Data->ScriptInstances.find(uuid) == s_Data->ScriptInstances.end())
		{
			GE_CORE_WARN("Script Instance not found. Returning nullptr.");
			return nullptr;
		}

		return s_Data->ScriptInstances.at(uuid)->GetMonoObject();
	}

	void Scripting::PrintMonoAssemblyTypes(MonoAssembly* assembly)
	{
		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionTable);

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			printf("%s.%s", nameSpace, name);
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
		return mono_string_new(s_Data->AppDomain, string);
	}

	std::string Scripting::MonoStringToString(MonoString* string)
	{
		char* cStr = mono_string_to_utf8(string);
		std::string str(cStr);
		mono_free(cStr);
		return str;
	}

	ScriptField::Type Scripting::StringToScriptFieldType(std::string_view type)
	{
		if (type == "None")		return ScriptField::Type::None;
		if (type == "Char")		return ScriptField::Type::Char;
		if (type == "Int")		return ScriptField::Type::Int;
		if (type == "UInt")		return ScriptField::Type::UInt;
		if (type == "Float")	return ScriptField::Type::Float;
		if (type == "Byte")		return ScriptField::Type::Byte;
		if (type == "Bool")		return ScriptField::Type::Bool;
		if (type == "Vector2")	return ScriptField::Type::Vector3;
		if (type == "Vector3")	return ScriptField::Type::Vector3;
		if (type == "Vector4")	return ScriptField::Type::Vector4;
		if (type == "Entity")	return ScriptField::Type::Entity;

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

	bool Scripting::ScriptClassExists(const std::string& fullName)
	{
		return s_Data->ScriptClasses.find(fullName) != s_Data->ScriptClasses.end();
	}

	void Scripting::Init()
	{
		GE_CORE_INFO("Scripting Init Start");

		InitMono();
		ScriptGlue::RegisterFunctions();

		LoadAssembly(Project::GetPathToAsset("scripts/Resources/Binaries/GE-ScriptCore.dll"));
		LoadApplicationAssembly(Project::GetPathToAsset(Project::GetConfig().ScriptPath));

		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();

		s_Data->EntityClass = ScriptClass("GE", "Entity", true);

		GE_CORE_INFO("Scripting Init Complete");
	}

	void Scripting::Shutdown()
	{
		GE_CORE_INFO("Scripting Shutdown Start");
		ShutdownMono();

		delete s_Data;
		GE_CORE_INFO("Scripting Shutdown Complete");
	}

	void Scripting::InitMono()
	{
		if (s_Data->RootDomain)
			return;

		GE_CORE_INFO("Mono Init Start");
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("GEJITRuntime");
		GE_CORE_ASSERT(rootDomain, "Mono Scripting initialization failure.");

		s_Data->RootDomain = rootDomain;

		mono_thread_set_main(mono_thread_current());

		GE_CORE_INFO("Mono Init Complete");
	}

	void Scripting::ShutdownMono()
	{
		GE_CORE_INFO("Mono Shutdown Start");
		if (s_Data->AppDomain && s_Data->RootDomain)
		{
			mono_domain_set(mono_get_root_domain(), false);
			mono_domain_unload(s_Data->AppDomain);
			s_Data->AppDomain = nullptr;

			// TODO : Fix
			//mono_jit_cleanup(s_Data->RootDomain);
			s_Data->RootDomain = nullptr;
		}
		GE_CORE_INFO("Mono Shutdown Complete");

	}

	void Scripting::OnStop()
	{
		s_Data->ActiveScene = nullptr;
		if(!s_Data->ScriptInstances.empty())
			s_Data->ScriptInstances.clear();
	}

	void Scripting::OnCreateScript(Entity entity)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();
		if (ScriptClassExists(sc.ClassName))
		{
			UUID uuid = entity.GetComponent<IDComponent>().ID;
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_Data->ScriptClasses.at(sc.ClassName), uuid);
			s_Data->ScriptInstances.emplace(uuid, instance);

			// Copy Field values if found
			if (s_Data->ScriptFields.find(uuid) != s_Data->ScriptFields.end())
			{
				ScriptFieldMap& fieldMap = s_Data->ScriptFields.at(uuid);
				for (auto& [name, fieldInstance] : fieldMap)
				{
					instance->SetFieldValue(name, fieldInstance.m_ValueBuffer);
				}
			}
			else
			{
				s_Data->ScriptFields.at(uuid) = (ScriptFieldMap&)( sc.ClassName, instance );
			}

			instance->InvokeOnCreate();
		}
	}

	void Scripting::OnUpdateScript(Entity entity, float timestep)
	{
		UUID uuid = entity.GetComponent<IDComponent>().ID;
		if (!uuid || s_Data->ScriptInstances.find(uuid) == s_Data->ScriptInstances.end())
		{
			GE_CORE_WARN("Tried to Update Entity/Script Instance that does not exist");
			return;
		}

		s_Data->ScriptInstances.at(uuid)->InvokeOnUpdate(timestep);
	}

	MonoObject* Scripting::InstantiateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
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

	void Scripting::LoadApplicationAssembly(const std::filesystem::path& filepath)
	{
		s_Data->AppAssemblyFilepath = filepath;
		s_Data->AppAssembly = LoadMonoAssembly(filepath);
		s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);
		//PrintMonoAssemblyTypes(s_Data->AppAssembly);

		// FileWatch setup - watches given app assembly filepath and call event
		s_Data->AppAssemblyFileWatcher =
				CreateScope<filewatch::FileWatch<std::string>>(filepath.string(), OnApplicationAssemblyFileSystemEvent);
		s_Data->AssemblyReloadPending = false;
	}

	void Scripting::LoadAssembly(const std::filesystem::path& filepath)
	{
		// Create an App Domain
		s_Data->AppDomain = mono_domain_create_appdomain("GEScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssemblyFilepath = filepath;
		s_Data->CoreAssembly = LoadMonoAssembly(filepath);
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		//PrintMonoAssemblyTypes(s_Data->CoreAssembly);
	}
	
	void Scripting::LoadAssemblyClasses()
	{
		s_Data->ScriptClasses.clear();

		const MonoTableInfo* typeDefinitionTable = mono_image_get_table_info(s_Data->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionTable);
		MonoClass* entityClass = mono_class_from_name(s_Data->CoreAssemblyImage, "GE", "Entity");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, name);
			else
				fullName = name;

			MonoClass* monoClass = mono_class_from_name(s_Data->AppAssemblyImage, nameSpace, name);
			bool isSubClass = mono_class_is_subclass_of(monoClass, entityClass, false);
			if (!isSubClass)
				continue;

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, name);
			s_Data->ScriptClasses[fullName] = scriptClass;

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

					scriptClass->m_Fields[fieldName] = ScriptField{ fieldTypeName, fieldType, monoField };

				}
			}
		}

	}

	void Scripting::ReloadAssembly()
	{
		GE_CORE_TRACE("Assembly Reload Started.");
		s_Data->AppAssemblyFileWatcher.reset();

		mono_domain_set(mono_get_root_domain(), false); // Set domain to root domain

		mono_domain_unload(s_Data->AppDomain); // Unload old app domain

		// Reload assemblies
		LoadAssembly(s_Data->CoreAssemblyFilepath);
		LoadApplicationAssembly(s_Data->AppAssemblyFilepath);
		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();

		s_Data->EntityClass = ScriptClass("GE", "Entity", true);
		GE_CORE_TRACE("Assembly Reload Complete");
	}

	void Scripting::OnApplicationAssemblyFileSystemEvent(const std::string& path, const filewatch::Event changeType)
	{
		if (!s_Data->AssemblyReloadPending && changeType == filewatch::Event::modified)
		{
			s_Data->AssemblyReloadPending = true;

			Application::SubmitToMainAppThread([]()
				{	
					ReloadAssembly();
				});
		}
	}
#pragma endregion

#pragma region ScriptGlue

	void ScriptGlue::RegisterFunctions()
	{
		GE_ADD_INTERNAL_CALL(Log_Core_Info);
		GE_ADD_INTERNAL_CALL(Log_Core_Trace);
		GE_ADD_INTERNAL_CALL(Log_Core_Warn);
		GE_ADD_INTERNAL_CALL(Log_Core_Error);
		GE_ADD_INTERNAL_CALL(Log_Core_Assert);

		GE_ADD_INTERNAL_CALL(Entity_HasComponent);
		GE_ADD_INTERNAL_CALL(Entity_FindEntityByTag);
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
	}

	template<typename T>
	static void RegisterComponent()
	{
		std::string typeName = typeid(T).name();
		size_t namePosition = typeName.find_last_of(':');
		std::string name = typeName.substr(namePosition + 1);
		std::string managedTypeName = fmt::format("GE.{}", name); // Namespace.ComponentName

		MonoType* managedType = mono_reflection_type_from_name(managedTypeName.data(), Scripting::s_Data->CoreAssemblyImage);
		if (!managedType) { GE_CORE_ERROR("Could not Register Component. Component Type not found."); return; }
		s_HasComponentsFuncs[managedType] = [](Entity entity) { return entity.HasComponent<T>(); };
	}

	void ScriptGlue::RegisterComponents()
	{
		s_HasComponentsFuncs.clear();

		//RegisterComponent<IDComponent>();
		//RegisterComponent<TagComponent>();
		RegisterComponent<TransformComponent>();
		RegisterComponent<AudioSourceComponent>();
		//RegisterComponent<AudioListenerComponent>();
		//RegisterComponent<SpriteRendererComponent>();
		//RegisterComponent<CircleRendererComponent>();
		RegisterComponent<TextRendererComponent>();
		//RegisterComponent<CameraComponent>();
		RegisterComponent<Rigidbody2DComponent>();
		//RegisterComponent<BoxCollider2DComponent>();
		//RegisterComponent<CircleCollider2DComponent>();
	}
#pragma endregion
}