#include "GE/GEpch.h"

#include "Scripting.h"
#include "GE/Core/Input/Input.h"
#include "GE/Core/Input/KeyCodes.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>

#include <box2d/b2_body.h>
#include <glm/glm.hpp>


namespace GE
{
	static std::unordered_map<MonoType*, std::function<bool(Entity)>>  s_HasComponentsFuncs;

	static std::unordered_map<std::string, ScriptFieldType>  s_ScriptFieldTypeNames
	{
		{ "System.Char",	ScriptFieldType::Char		},
		{ "System.Int",		ScriptFieldType::Int		},
		{ "System.UInt",	ScriptFieldType::UInt		},
		{ "System.Single",	ScriptFieldType::Float		},
		{ "System.Byte",	ScriptFieldType::Byte		},
		{ "System.Boolean",	ScriptFieldType::Bool		},
		{ "GE.Vector2",		ScriptFieldType::Vector2	},
		{ "GE.Vector3",		ScriptFieldType::Vector3	},
		{ "GE.Vector4",		ScriptFieldType::Vector4	},
		{ "GE.Entity",		ScriptFieldType::Entity		}
	};

#define GE_ADD_INTERNAL_CALL(Name) mono_add_internal_call("GE.InternalCalls::" #Name, Name)

#pragma region Interal Call Declarations
#pragma region Log

	static void Log_Core_Info(MonoString* debugMessage)
	{
		Scene* scene = Scripting::GetScene();
		GE_CORE_ASSERT(scene, "Scene is Undefined.");

		char* cStr = mono_string_to_utf8(debugMessage);
		GE_CORE_INFO(cStr);
		mono_free(cStr);

	}

	static void Log_Core_Trace(MonoString* debugMessage)
	{
		Scene* scene = Scripting::GetScene();
		GE_CORE_ASSERT(scene, "Scene is Undefined.");

		char* cStr = mono_string_to_utf8(debugMessage);
		GE_CORE_TRACE(cStr);
		mono_free(cStr);

	}

	static void Log_Core_Warn(MonoString* debugMessage)
	{
		Scene* scene = Scripting::GetScene();
		GE_CORE_ASSERT(scene, "Scene is Undefined.");

		char* cStr = mono_string_to_utf8(debugMessage);
		GE_CORE_WARN(cStr);
		mono_free(cStr);

	}

	static void Log_Core_Error(MonoString* debugMessage)
	{
		Scene* scene = Scripting::GetScene();
		GE_CORE_ASSERT(scene, "Scene is Undefined.");

		char* cStr = mono_string_to_utf8(debugMessage);
		GE_CORE_ERROR(cStr);
		mono_free(cStr);

	}

	static void Log_Core_Assert(MonoObject* object, MonoString* debugMessage)
	{
		Scene* scene = Scripting::GetScene();
		GE_CORE_ASSERT(scene, "Scene is Undefined.");

		char* cStr = mono_string_to_utf8(debugMessage);
		GE_CORE_ASSERT(!object, cStr);
		mono_free(cStr);

	}


#pragma endregion

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

	static UUID Entity_FindEntityByTag(MonoString* tagString)
	{
		Scene* scene = Scripting::GetScene();
		GE_CORE_ASSERT(scene, "Scene is Undefined.");

		char* cTag = mono_string_to_utf8(tagString);
		GE_CORE_INFO("Name = " + std::string(cTag));
		Entity entity = scene->GetEntityByTag(cTag);
		mono_free(cTag);

		if (entity == Entity{})
		{
			GE_CORE_INFO("Entity undefined. Returning 0.");
			return 0;
		}
		GE_CORE_INFO("Entity Found. Returning UUID = " + entity.GetUUID());
		return entity.GetUUID();
	}
	
	static MonoObject* Entity_GetScriptInstance(uint64_t uuid)
	{
		return Scripting::GetObjectInstance(uuid);
	}

	static bool Input_IsKeyDown(KeyCode keyCode)
	{
		return Input::IsKeyPressed(keyCode);
	}
#pragma endregion

	static void PrintMonoAssemblyTypes(MonoAssembly* assembly)
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

	static ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
	{
		const char* typeName = mono_type_get_name(monoType);
		//GE_CORE_INFO(typeName);
		ScriptFieldType type = s_ScriptFieldTypeNames.at(typeName);
		//GE_CORE_INFO(ScriptFieldTypeToString(type));

		if (s_ScriptFieldTypeNames.find(typeName) == s_ScriptFieldTypeNames.end())
			return ScriptFieldType::None;

		return s_ScriptFieldTypeNames.at(typeName);
	}

	struct ScriptingData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;
		MonoAssembly* ApplicationAssembly = nullptr;
		MonoImage* ApplicationAssemblyImage = nullptr;

		std::filesystem::path CoreAssemblyFilepath;
		std::filesystem::path ApplicationAssemblyFilepath;

		Scope<filewatch::FileWatch<std::string>> ApplicationAssemblyFileWatcher;
		bool AssemblyReloadPending = false;

		ScriptClass EntityClass;

		std::unordered_map<std::string, Ref<ScriptClass>> ScriptClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> ScriptInstances;

		std::unordered_map<UUID, ScriptFieldMap> ScriptFields;

		Scene* SceneContext = nullptr;

	};

	static ScriptingData* s_ScriptingData;

#pragma region ScriptClass

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(isCore ? s_ScriptingData->CoreAssemblyImage : s_ScriptingData->ApplicationAssemblyImage, classNamespace.c_str(), className.c_str());
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
		return mono_runtime_invoke(method, instance, params, &exception);
	}

#pragma endregion

#pragma region ScriptInstance
	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, UUID uuid) : m_ScriptClass(scriptClass)
	{
		m_Instance = m_ScriptClass->Instantiate();

		m_Constructor = s_ScriptingData->EntityClass.GetMethod(".ctor", 1);
		m_OnCreate = m_ScriptClass->GetMethod("OnCreate", 0);
		m_OnUpdate = m_ScriptClass->GetMethod("OnUpdate", 1);

		{
			void* param = &uuid;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
		}
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (!m_OnCreate || !m_Instance)
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
			return;

		const ScriptField& field = instance->second;
		mono_field_get_value(m_Instance, field.Field, buffer);
	}

	void ScriptInstance::Internal_SetFieldValue(const std::string& name, const void* value)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto instance = fields.find(name);
		if (instance == fields.end())
			return;

		const ScriptField& field = instance->second;
		mono_field_set_value(m_Instance, field.Field, (void*)value);
	}
#pragma endregion

#pragma region Scripting

	void Scripting::SetScene(Scene* scene)
	{
		s_ScriptingData->SceneContext = scene;
	}

	Scene* Scripting::GetScene()
	{
		return s_ScriptingData->SceneContext;
	}

	Ref<ScriptInstance> Scripting::GetScriptInstance(UUID uuid)
	{
		auto instance = s_ScriptingData->ScriptInstances.find(uuid);
		if (instance == s_ScriptingData->ScriptInstances.end())
			return nullptr;
		return instance->second;
	}

	Ref<ScriptClass> Scripting::GetScriptClass(const std::string& name)
	{
		auto scriptClass = s_ScriptingData->ScriptClasses.find(name);
		if (scriptClass == s_ScriptingData->ScriptClasses.end())
			return nullptr;
		return scriptClass->second;
	}

	ScriptFieldMap& Scripting::GetScriptFieldMap(Entity entity)
	{
		GE_CORE_ASSERT(entity, "Entity does not exist.");

		UUID uuid = entity.GetUUID();

		return s_ScriptingData->ScriptFields[uuid];
	}

	MonoObject* Scripting::GetObjectInstance(UUID uuid)
	{
		//GE_CORE_ASSERT(s_ScriptingData->ScriptInstances.find(uuid) != s_ScriptingData->ScriptInstances.end(), "Script Instance does not exist for given UUID.");
		if (s_ScriptingData->ScriptInstances.find(uuid) == s_ScriptingData->ScriptInstances.end())
		{
			GE_CORE_WARN("Script Instance not found. Returning null.");
			return nullptr;
		}

		return s_ScriptingData->ScriptInstances.at(uuid)->GetMonoObject();
	}

	bool Scripting::ScriptClassExists(const std::string& fullName)
	{
		return s_ScriptingData->ScriptClasses.find(fullName) != s_ScriptingData->ScriptClasses.end();
	}

	void Scripting::Init()
	{
		s_ScriptingData = new ScriptingData();

		InitMono();
		ScriptGlue::RegisterFunctions();

		LoadAssembly("projects/demo/assets/Resources/Binaries/GE-ScriptCore.dll");
		LoadApplicationAssembly("projects/demo/assets/Resources/Binaries/demo.dll");
		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();

		s_ScriptingData->EntityClass = ScriptClass("GE", "Entity", true);

	}

	void Scripting::Shutdown()
	{
		ShutdownMono();

		delete s_ScriptingData;
	}

	void Scripting::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("GEJITRuntime");
		GE_CORE_ASSERT(rootDomain, "Mono Scripting initialization failure.");

		s_ScriptingData->RootDomain = rootDomain;

		mono_thread_set_main(mono_thread_current());
	}

	void Scripting::ShutdownMono()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_ScriptingData->AppDomain);
		s_ScriptingData->AppDomain = nullptr;

		mono_jit_cleanup(s_ScriptingData->RootDomain);
		s_ScriptingData->RootDomain = nullptr;

	}

	void Scripting::OnStop()
	{
		s_ScriptingData->SceneContext = nullptr;
		s_ScriptingData->ScriptClasses.clear();
		s_ScriptingData->ScriptInstances.clear();
	}

	void Scripting::OnCreateScript(Entity entity)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();
		if (ScriptClassExists(sc.ClassName))
		{
			UUID uuid = entity.GetUUID();
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_ScriptingData->ScriptClasses[sc.ClassName], uuid);
			s_ScriptingData->ScriptInstances[uuid] = instance;

			// Copy Field values if found
			if (s_ScriptingData->ScriptFields.find(uuid) != s_ScriptingData->ScriptFields.end())
			{
				ScriptFieldMap& fieldMap = s_ScriptingData->ScriptFields.at(uuid);
				for (auto& [name, fieldInstance] : fieldMap)
				{
					instance->SetFieldValue(name, fieldInstance.m_ValueBuffer);
				}
			}
			else
			{
				s_ScriptingData->ScriptFields[uuid] = (ScriptFieldMap&)( sc.ClassName, instance );
			}

			instance->InvokeOnCreate();
		}
	}

	void Scripting::OnUpdateScript(Entity entity, float timestep)
	{
		UUID uuid = entity.GetUUID();
		//GE_CORE_ASSERT(s_ScriptingData->ScriptInstances.find(uuid) != s_ScriptingData->ScriptInstances.end(), "Cannot find Entity in Script Instances.");
		if (!uuid || s_ScriptingData->ScriptInstances.find(uuid) == s_ScriptingData->ScriptInstances.end())
			return;

		s_ScriptingData->ScriptInstances[uuid]->InvokeOnUpdate(timestep);
	}

	MonoObject* Scripting::InstantiateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_ScriptingData->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	MonoAssembly* Scripting::LoadMonoAssembly(const std::filesystem::path& assemblyPath)
	{
		uint32_t fileSize = 0;
		char* fileData = ReadBytes(assemblyPath, &fileSize);

		MonoImageOpenStatus monoStatus;
		MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &monoStatus, 0);
		delete[] fileData;

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
		s_ScriptingData->ApplicationAssemblyFilepath = filepath;
		s_ScriptingData->ApplicationAssembly = LoadMonoAssembly(filepath);
		s_ScriptingData->ApplicationAssemblyImage = mono_assembly_get_image(s_ScriptingData->ApplicationAssembly);
		//PrintMonoAssemblyTypes(s_ScriptingData->ApplicationAssembly);

		// FileWatch setup - watches given app assembly filepath and call event
		s_ScriptingData->ApplicationAssemblyFileWatcher =
				CreateScope<filewatch::FileWatch<std::string>>(filepath.string(), OnApplicationAssemblyFileSystemEvent);
		s_ScriptingData->AssemblyReloadPending = false;
	}

	void Scripting::LoadAssembly(const std::filesystem::path& filepath)
	{
		// Create an App Domain
		s_ScriptingData->AppDomain = mono_domain_create_appdomain("GEScriptRuntime", nullptr);
		mono_domain_set(s_ScriptingData->AppDomain, true);

		s_ScriptingData->CoreAssemblyFilepath = filepath;
		s_ScriptingData->CoreAssembly = LoadMonoAssembly(filepath);
		s_ScriptingData->CoreAssemblyImage = mono_assembly_get_image(s_ScriptingData->CoreAssembly);
		//PrintMonoAssemblyTypes(s_ScriptingData->CoreAssembly);
	}
	void Scripting::LoadAssemblyClasses()
	{
		s_ScriptingData->ScriptClasses.clear();

		const MonoTableInfo* typeDefinitionTable = mono_image_get_table_info(s_ScriptingData->ApplicationAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionTable);
		MonoClass* entityClass = mono_class_from_name(s_ScriptingData->CoreAssemblyImage, "GE", "Entity");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(s_ScriptingData->ApplicationAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(s_ScriptingData->ApplicationAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, name);
			else
				fullName = name;

			MonoClass* monoClass = mono_class_from_name(s_ScriptingData->ApplicationAssemblyImage, nameSpace, name);
			bool isSubClass = mono_class_is_subclass_of(monoClass, entityClass, false);
			if (!isSubClass)
				continue;

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, name);
			s_ScriptingData->ScriptClasses[fullName] = scriptClass;

			void* iterator = nullptr;
			while (MonoClassField* monoField = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(monoField);

				uint32_t flags = mono_field_get_flags(monoField);
				if (flags & MONO_FIELD_ATTR_PUBLIC)
				{
					MonoType* monoType = mono_field_get_type(monoField);

					ScriptFieldType fieldType = MonoTypeToScriptFieldType(monoType);
					const char* fieldTypeName = ScriptFieldTypeToString(fieldType);

					scriptClass->m_Fields[fieldName] = ScriptField{ fieldTypeName, fieldType, monoField };

				}
			}
		}

	}

	void Scripting::ReloadAssembly()
	{
		GE_CORE_TRACE("Assembly Reload Started.");
		s_ScriptingData->ApplicationAssemblyFileWatcher.reset();

		mono_domain_set(mono_get_root_domain(), false); // Set domain to root domain

		mono_domain_unload(s_ScriptingData->AppDomain); // Unload old app domain

		// Reload assemblies
		LoadAssembly(s_ScriptingData->CoreAssemblyFilepath);
		LoadApplicationAssembly(s_ScriptingData->ApplicationAssemblyFilepath);
		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();

		s_ScriptingData->EntityClass = ScriptClass("GE", "Entity", true);
		GE_CORE_TRACE("Assembly Reload Complete");
	}

	char* Scripting::ReadBytes(const std::filesystem::path& filePath, uint32_t* fileSize)
	{
		std::ifstream stream(filePath, std::ios::binary | std::ios::ate);
		if (!stream)
		{
			GE_CORE_ASSERT(false, "Failed to open scripting file.");
			return nullptr;
		}

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint32_t size = end - stream.tellg();
		if (size == 0)
		{
			GE_CORE_ASSERT(false, "File is empty.");
			return nullptr;
		}

		char* buffer = new char[size];
		stream.read(buffer, size);
		stream.close();

		*fileSize = size;
		return buffer;
	}
	
	void Scripting::OnApplicationAssemblyFileSystemEvent(const std::string& path, const filewatch::Event changeType)
	{
		if (!s_ScriptingData->AssemblyReloadPending && changeType == filewatch::Event::modified)
		{
			s_ScriptingData->AssemblyReloadPending = true;

			Application::GetApplication().SubmitToMainThread([]()
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

		GE_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		GE_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);

		GE_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		GE_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);

		GE_ADD_INTERNAL_CALL(Entity_HasComponent);
		GE_ADD_INTERNAL_CALL(Entity_FindEntityByTag);
		GE_ADD_INTERNAL_CALL(Entity_GetScriptInstance);

		GE_ADD_INTERNAL_CALL(Input_IsKeyDown);
	}

	template<typename T>
	static void RegisterComponent()
	{
		std::string typeName = typeid(T).name();
		size_t namePosition = typeName.find_last_of(':');
		std::string name = typeName.substr(namePosition + 1);
		std::string managedTypeName = fmt::format("GE.{}", name); // Namespace.ComponentName

		MonoType* managedType = mono_reflection_type_from_name(managedTypeName.data(), s_ScriptingData->CoreAssemblyImage);
		if (!managedType) { GE_CORE_ERROR("Could not Register Component. Component Type not found."); return; }
		s_HasComponentsFuncs[managedType] = [](Entity entity) { return entity.HasComponent<T>(); };
	}

	void ScriptGlue::RegisterComponents()
	{
		s_HasComponentsFuncs.clear();

		//RegisterComponent<IDComponent>();
		//RegisterComponent<TagComponent>();
		RegisterComponent<TransformComponent>();
		//RegisterComponent<SpriteRendererComponent>();
		//RegisterComponent<CircleRendererComponent>();
		//RegisterComponent<CameraComponent>();
		RegisterComponent<Rigidbody2DComponent>();
		//RegisterComponent<BoxCollider2DComponent>();
		//RegisterComponent<CircleCollider2DComponent>();
	}
#pragma endregion
}