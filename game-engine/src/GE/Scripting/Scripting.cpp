#include "GE/GEpch.h"

#include "Scripting.h"
#include "GE/Core/Input/Input.h"
#include "GE/Core/Input/KeyCodes.h"

#include "mono/metadata/object.h"
#include "glm/glm.hpp"
#include <box2d/b2_body.h>

namespace GE
{
	static std::unordered_map<MonoType*, std::function<bool(Entity)>>  s_HasComponentsFuncs;

#define GE_ADD_INTERNAL_CALL(Name) mono_add_internal_call("GE.InternalCalls::" #Name, Name)

	static void TransformComponent_GetTranslation(UUID uuid, glm::vec3* translation)
	{
		Scene* scene = Scripting::GetScene();
		
		Entity entity = scene->GetEntity(uuid);
		if (entity.HasComponent<TransformComponent>())
		{
			auto& tc = entity.GetComponent<TransformComponent>();
			*translation = tc.Translation;
		}
	}

	static void TransformComponent_SetTranslation(UUID uuid, glm::vec3* translation)
	{
		Scene* scene = Scripting::GetScene();

		Entity entity = scene->GetEntity(uuid);
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
		Entity entity = scene->GetEntity(uuid);
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
		Entity entity = scene->GetEntity(uuid);
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
		Entity entity = scene->GetEntity(uuid);
		GE_CORE_ASSERT(entity, "Entity is Undefined.");

		MonoType* type = mono_reflection_type_get_type(componentType);
		GE_CORE_ASSERT(s_HasComponentsFuncs.find(type) != s_HasComponentsFuncs.end(), "Unable to find Component Type.");
		return s_HasComponentsFuncs.at(type)(entity);
	}

	static bool Input_IsKeyDown(KeyCode keyCode)
	{
		return Input::IsKeyPressed(keyCode);
	}

	static void PrintAssemblyTypes(MonoAssembly* assembly)
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

	struct ScriptingData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		ScriptClass EntityClass;

		std::unordered_map<std::string, Ref<ScriptClass>> ScriptClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> ScriptInstances;

		Scene* SceneContext = nullptr;
	};

	static ScriptingData* s_Data;

	void Scripting::Init()
	{
		s_Data = new ScriptingData();

		InitMono();

		LoadAssembly("Resources/Scripts/GE-ScriptCore.dll");
		LoadAssemblyClasses(s_Data->CoreAssembly);

		ScriptGlue::RegisterComponents();
		ScriptGlue::RegisterFunctions();
		
		s_Data->EntityClass = ScriptClass("GE", "Entity");
		
	}

	void Scripting::Shutdown()
	{
		// Mono is hard to shutdown
		//ShutdownMono();
		 
		delete s_Data;
	}
	
	void Scripting::SetScene(Scene* scene)
	{
		s_Data->SceneContext = scene;
	}
	
	Scene* Scripting::GetScene()
	{
		return s_Data->SceneContext;
	}
	
	void Scripting::OnStop()
	{
		s_Data->SceneContext = nullptr;
		s_Data->ScriptClasses.clear();
		s_Data->ScriptInstances.clear();
	}

	bool Scripting::ScriptClassExists(const std::string& fullName)
	{
		return s_Data->ScriptClasses.find(fullName) != s_Data->ScriptClasses.end();
	}

	void Scripting::OnCreateScript(Entity entity)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();
		if (ScriptClassExists(sc.ClassName))
		{
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_Data->ScriptClasses[sc.ClassName], entity.GetUUID());
			s_Data->ScriptInstances[entity.GetUUID()] = instance;
			instance->InvokeOnCreate();
		}
	}

	void Scripting::OnUpdateScript(Entity entity, float timestep)
	{
		UUID uuid = entity.GetUUID();
		GE_CORE_ASSERT(s_Data->ScriptInstances.find(uuid) != s_Data->ScriptInstances.end(), "Cannot find Entity in Script Instances.");
		
		s_Data->ScriptInstances[uuid]->InvokeOnUpdate(timestep);
	}

	void Scripting::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("GEJITRuntime");
		if (rootDomain == nullptr)
		{
			GE_CORE_ERROR("Mono Scripting initialization failure.");
			return;
		}

		s_Data->RootDomain = rootDomain;
	}

	void Scripting::ShutdownMono()
	{
		mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;

		mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;
	}

	MonoObject* Scripting::InstantiateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
	}
	
	MonoAssembly* Scripting::LoadMonoAssembly(const std::filesystem::path& assemblyPath)
	{
		uint32_t fileSize = 0;
		char* fileData = ReadBytes(assemblyPath, &fileSize);

		MonoImageOpenStatus monoStatus;
		MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &monoStatus, 0);

		if (monoStatus != MONO_IMAGE_OK)
		{
			const char* errorMessage = mono_image_strerror(monoStatus);
			GE_CORE_ASSERT(false, errorMessage);
			return nullptr;
		}

		std::string pathString = assemblyPath.string();
		MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &monoStatus, 0);
		mono_image_close(image);

		delete[] fileData;

		return assembly;
	}

	void Scripting::LoadAssembly(const std::filesystem::path& filepath)
	{
		// Create an App Domain
		s_Data->AppDomain = mono_domain_create_appdomain("GEScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		// Move this maybe
		s_Data->CoreAssembly = LoadMonoAssembly(filepath);
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		///PrintAssemblyTypes(s_Data->CoreAssembly);
	}

	void Scripting::LoadAssemblyClasses(MonoAssembly* assembly)
	{
		s_Data->ScriptClasses.clear();

		MonoImage* image = mono_assembly_get_image(assembly);

		const MonoTableInfo* typeDefinitionTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionTable);
		MonoClass* entityClass = mono_class_from_name(image, "GE", "Entity");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, name);
			else
				fullName = name;

			MonoClass* monoClass = mono_class_from_name(image, nameSpace, name);
			bool isSubClass = mono_class_is_subclass_of(monoClass, entityClass, false);
			if (isSubClass)
				s_Data->ScriptClasses[fullName] = CreateRef<ScriptClass>(nameSpace, name);
		}
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

#pragma region ScriptClass

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(s_Data->CoreAssemblyImage, classNamespace.c_str(), className.c_str());
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
		return mono_runtime_invoke(method, instance, params, nullptr);
	}

#pragma endregion

#pragma region ScriptInstance
	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, UUID uuid) : m_ScriptClass(scriptClass)
	{
		m_Instance = m_ScriptClass->Instantiate();

		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);
		m_OnCreate = m_ScriptClass->GetMethod("OnCreate", 0);
		m_OnUpdate = m_ScriptClass->GetMethod("OnUpdate", 1);

		{
			void* param = &uuid;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
		}
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (m_OnCreate == nullptr || m_Instance == nullptr)
			return;
		m_ScriptClass->InvokeMethod(m_Instance, m_OnCreate);
	}

	void ScriptInstance::InvokeOnUpdate(float timestep)
	{
		if (m_Instance == nullptr)
			return;

		void* param = &timestep;
		m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdate, &param);
	}

#pragma endregion

	void ScriptGlue::RegisterFunctions()
	{
		GE_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		GE_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);

		GE_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		GE_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);

		GE_ADD_INTERNAL_CALL(Entity_HasComponent);

		GE_ADD_INTERNAL_CALL(Input_IsKeyDown);
	}

	template<typename T>
	static void RegisterComponent()
	{
		std::string typeName = typeid(T).name();
		size_t namePosition = typeName.find_last_of(':');
		std::string name = typeName.substr(namePosition + 1);
		std::string managedTypeName = fmt::format("GE.{}", name); // Namespace.ComponentName
		GE_CORE_INFO(managedTypeName);

		MonoType* managedType = mono_reflection_type_from_name(managedTypeName.data(), s_Data->CoreAssemblyImage);
		if (!managedType) { GE_CORE_ERROR("Could not find Component Type."); return; }
		s_HasComponentsFuncs[managedType] = [](Entity entity) { return entity.HasComponent<T>(); };
	}

	void ScriptGlue::RegisterComponents()
	{
		RegisterComponent<TransformComponent>();
		RegisterComponent<Rigidbody2DComponent>();
	}

}