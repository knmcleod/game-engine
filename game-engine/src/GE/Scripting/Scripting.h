#pragma once

#include "ScriptingUtils.h"
#include "GE/Asset/Assets/Scene/Entity.h"
#include "GE/Asset/Assets/Script/Script.h"

#include "GE/Core/Events/Event.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <FileWatch.h>
#pragma warning(pop)

#include <glm/glm.hpp>

namespace GE
{
	// Forward declarations
	class Scene;

	class ScriptInstance
	{
		friend class Scripting;
	public:
		ScriptInstance(UUID scriptHandle, UUID entityID);
		~ScriptInstance() = default;

		MonoObject* GetMonoObject() { return m_Instance; }

		template<typename T>
		const T GetFieldValue(const std::string& name)
		{
			T data = T();
			Internal_GetFieldValue(name, (void*)&data);
			return data;
		}

		template<typename T>
		void SetFieldValue(const std::string& name, const T& data)
		{
			Internal_SetFieldValue(name, &data);
		}

	private:
		void InvokeOnCreate();
		void InvokeOnUpdate(float timestep);
		bool InvokeOnEvent(Event& e);

		void Internal_GetFieldValue(const std::string& name, void* buffer);
		void Internal_SetFieldValue(const std::string& name, const void* value);
	private:
		UUID m_ScriptHandle = 0;

		MonoObject* m_Instance = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreate = nullptr;
		MonoMethod* m_OnUpdate = nullptr;
		MonoMethod* m_OnEvent = nullptr;
		MonoMethod* m_OnKeyPressedEvent = nullptr;
		MonoMethod* m_OnMousePressedEvent = nullptr;
	};

	class Scripting
	{
	public:
		/*
		* ClassName::MethodName, ParamCount
		*/
		using MethodMap = std::unordered_map<std::string, uint32_t>;
		using Method = std::pair<std::string, uint32_t>;

		struct Data
		{
			friend class Scripting;
		public:
			Data()
			{

			}

			~Data()
			{
				Scripts.clear();
				Scripts = std::unordered_map<std::string, Ref<Script>>();

				for (auto& [uuid, instanceFieldMap] : EntityScriptFields)
				{
					for (auto& [name, instanceField] : instanceFieldMap)
					{
						instanceField.m_DataBuffer.Release();
					}
				}
				EntityScriptFields.clear();
				EntityScriptFields = std::unordered_map<UUID, ScriptFieldMap>();

				AllClassMethods.clear();
				AllClassMethods = std::unordered_map<UUID, MethodMap>();
			}
			
			const std::unordered_map<std::string, Ref<Script>>& GetScripts() { return Scripts; }

			bool ScriptExists(const std::string& className)
			{
				return Scripts.find(className) != s_Data.Scripts.end();
			}
			bool ScriptExists(UUID handle)
			{
				for (const auto& [fullname, script] : Scripts)
				{
					if (script->GetHandle() == handle)
						return true;
				}
				return false;
			}
			Ref<Script> GetScript(const std::string& className)
			{
				if (Scripts.find(className) != Scripts.end())
					return Scripts.at(className);
				GE_CORE_WARN("Scripting::Data::GetScript(const std::string&) - \n\tCannot find Script by className. Returning nullptr.");
				return nullptr;
			}
			Ref<Script> GetScript(UUID handle)
			{
				for (const auto& [className, script] : Scripts)
				{
					if (script->GetHandle() == handle)
						return script;
				}
				GE_CORE_WARN("Scripting::Data::GetScript(UUID) - \n\tCannot find Script by handle. Returning nullptr.");
				return nullptr;
			}

			bool EntityInstanceExists(UUID uuid)
			{
				return EntityScriptInstances.find(uuid) != EntityScriptInstances.end();
			}
			Ref<ScriptInstance> CreateEntityInstance(Ref<Script> script, UUID entityID)
			{
				if (!script)
					return nullptr;

				Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(script->GetHandle(), entityID);
				EntityScriptInstances.emplace(entityID, instance);

				// Script Fields
				{
					GE_PROFILE_SCOPE("Scripting::Data::CreateEntityInstance(UUID, UUID) - Script Fields");
					bool entityFieldsExist = s_Data.EntityScriptFields.find(entityID) != s_Data.EntityScriptFields.end();
					const ScriptFieldMap& fields = (entityFieldsExist ? s_Data.EntityScriptFields.at(entityID) : script->m_Fields);

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

				return instance;
			}

			/*
			* returns runtime instance
			* @param uuid : Entity IDComponent::ID
			*/
			Ref<ScriptInstance> GetEntityInstance(UUID uuid)
			{
				if (EntityInstanceExists(uuid))
					return EntityScriptInstances.at(uuid);
				GE_CORE_WARN("Scripting::Data::GetEntityInstance(UUID) - \n\tCannot find Script Instance by UUID. Returning nullptr.");
				return nullptr;
			}

			ScriptFieldMap& GetEntityFields(UUID uuid)
			{
				return s_Data.EntityScriptFields[uuid];
			}
			/*
			* Returns pointer to base Entity constructor with one param of type UUID.
			*/
			MonoMethod* GetEntityConstructor()
			{
				return Scripting::GetMethod(EntityScript, { ".ctor", 1 });
			}

			void SetScriptHandle(const std::string& className, UUID handle)
			{
				if (handle && !className.empty() && s_Data.Scripts.find(className) != s_Data.Scripts.end())
					s_Data.Scripts.at(className)->p_Handle = handle;
			}
			void AddScript(const std::string& className, Ref<Script> script)
			{
				s_Data.Scripts.emplace(className, script);
			}

			const MethodMap& GetMethods(UUID scriptHandle)
			{
				return AllClassMethods.at(scriptHandle);
			}
			void AddMethod(UUID scriptHandle, const MethodMap& methodInfo)
			{
				s_Data.AllClassMethods.emplace(scriptHandle, methodInfo);
			}
			bool ClearMethods(UUID scriptHandle)
			{
				if (AllClassMethods.find(scriptHandle) == AllClassMethods.end())
					return false;
				AllClassMethods.at(scriptHandle).clear();
				AllClassMethods.at(scriptHandle) = MethodMap();
				return true;
			}
			
			bool ClearInstances()
			{
				if (!s_Data.EntityScriptInstances.empty())
				{
					s_Data.EntityScriptInstances.clear();
					s_Data.EntityScriptInstances = std::unordered_map<UUID, Ref<ScriptInstance>>();
					return true;
				}
				return false;
			}
		
			void InstantiateBaseEntity()
			{
				EntityScript = CreateRef<Script>("GE", "Entity");
				Scripting::InstantiateScriptClass(EntityScript);
			}
		
			void ResetFileWatcher() { AppAssemblyFileWatcher.reset(); }

			bool ReloadPending() const { return AssemblyReloadPending; }
			void QueueReload() { AssemblyReloadPending = true; }

		private:
			void ClearDomains()
			{
				if (AppDomain)
				{
					Scripting::ClearAppDomain(AppDomain);
					AppDomain = nullptr;
				}

				if (CoreDomain)
				{
					Scripting::ClearCoreDomain(CoreDomain);
					CoreDomain = nullptr;
				}
			}
			void InitializeCoreDomain()
			{
				CoreDomain = Scripting::LoadCoreDomain("GEJITRuntime");
				GE_CORE_ASSERT(CoreDomain, "Mono Scripting Core Initialization failure.");

			}
			void InitializeAppDomain()
			{
				AppDomain = Scripting::LoadAppDomain("GEScriptRuntime");
			}

			void InitializeCore(const std::filesystem::path& filePath)
			{
				// Create an App Domain
				InitializeAppDomain();

				CoreAssembly = Scripting::LoadMonoAssembly(filePath);
				CoreAssemblyImage = Scripting::LoadMonoImage(CoreAssembly);
			}
			void InitializeApp(const std::filesystem::path& filePath)
			{
				AppAssembly = Scripting::LoadMonoAssembly(filePath);
				AppAssemblyImage = Scripting::LoadMonoImage(AppAssembly);

				// FileWatch setup - watches given app assembly filePath and call event
				AppAssemblyFileWatcher =
					CreateScope<filewatch::FileWatch<std::string>>(filePath.string(), OnFileSystemAppAssemblyEvent);
				AssemblyReloadPending = false;
			}

		private:

			MonoDomain* CoreDomain = nullptr;
			MonoDomain* AppDomain = nullptr;

			MonoAssembly* CoreAssembly = nullptr;
			MonoImage* CoreAssemblyImage = nullptr;
			MonoAssembly* AppAssembly = nullptr;
			MonoImage* AppAssemblyImage = nullptr;

			Scope<filewatch::FileWatch<std::string>> AppAssemblyFileWatcher;
			bool AssemblyReloadPending = false;

			// Base C# Entity Class
			Ref<Script> EntityScript;

			// <Fullname, Script Asset>
			std::unordered_map<std::string, Ref<Script>> Scripts;

			// <IDComponent.ID, RuntimeScript>
			std::unordered_map<UUID, Ref<ScriptInstance>> EntityScriptInstances;
			// <IDComponent.ID, DeserializedFields> 
			std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

			// Script Asset Handle, std::unordered_map<methodNameStr, paramCount>
			std::unordered_map<UUID, MethodMap> AllClassMethods = std::unordered_map<UUID, MethodMap>();
		};
#pragma region Helpers
		/*
		* Print to console all Types loaded from assembly
		*
		* @param assembly : example GE-ScriptCore(CoreAssembly)
		*/
		static void PrintMonoAssemblyTypes(MonoAssembly* assembly);
		/*
		* Converts internal field type to ScriptField::Type.
		* If not found, returns ScriptField::Type::None
		* @param monoType : type to convert
		*/
		static ScriptField::Type MonoTypeToScriptFieldType(MonoType* monoType);
		/*
		* Converts string to internal string type using Data::AppDomain.
		* @param string : example use, passing NameComponent::Name to C# Script.
		*/
		static MonoString* StringToMonoString(const char* string);
		/*
		* Converts internal string type to string
		* @param string : internal string type. example use, passing C# Script String to NameComponent::Name.
		*/
		static std::string MonoStringToString(MonoString* string);
		/*
		* Returns ScriptField::Type equivalent to string.
		* If not found, returns ScriptField::Type::None.
		* Could be changed to a map<ScriptField::Type, std::string> or similar.
		* @param typeStr : See ScriptField::Type for equivalents
		*/
		static ScriptField::Type StringToScriptFieldType(const std::string& typeStr);
		/*
		* Returns string equivalent to ScriptField::Type .
		* If not found, returns "<Invalid>".
		* @param fieldType : See ScriptField::Type for equivalents
		*/
		static const char* ScriptFieldTypeToString(ScriptField::Type fieldType);

#pragma endregion

#pragma region Data Extensions

		/*
		* Returns true if className key is found in Data::Scripts
		* @param className : example "className"
		*/
		static bool ScriptExists(const std::string& className) { return s_Data.ScriptExists(className); }
		/*
		* Returns true if handle is found in Data::Scripts
		* @param handle : Script Asset Handle
		*/
		static bool ScriptExists(UUID handle) { return s_Data.ScriptExists(handle); }

		/*
		* Returns Script value in Data::Scripts using className key.
		*
		* @param className : example "className"
		*/
		static Ref<Script> GetScript(const std::string& className) { return s_Data.GetScript(className); }
		/*
		* Returns Script value in Data::Scripts using handle.
		*
		* @param handle : Script Asset Handle
		*/
		static Ref<Script> GetScript(UUID handle) { return s_Data.GetScript(handle); }

		static const std::unordered_map<std::string, Ref<Script>>& GetScripts() { return s_Data.GetScripts(); }

		static const MethodMap& GetAllMethods(Ref<Script> script) { return s_Data.GetMethods(script->GetHandle()); }

		static void SetScriptHandle(const std::string& className, UUID handle) { s_Data.SetScriptHandle(className, handle); }

		/*
		* Returns Instance of Script value in Data::EntityScriptIntances using uuid key.
		* If not found, returns nullptr.
		* @param uuid : corresponds to Entity IDComponent::ID
		*/
		static Ref<ScriptInstance> GetEntityInstance(UUID uuid) { return s_Data.GetEntityInstance(uuid); }

		/*
		* Returns ScriptFieldMap value in Data::EntityScriptFields using uuid key.
		* Key doesn't have to exist.
		* @param uuid : corresponds to Entity IDComponent::ID
		*/
		static ScriptFieldMap& GetEntityFields(UUID uuid) { return s_Data.GetEntityFields(uuid); }

		static MonoMethod* GetEntityConstructor() { return s_Data.GetEntityConstructor(); }
#pragma endregion

		/*
		* Returns true if HasComponentFunc is found for type.
		*
		* @param type : Any internal Component type
		*/
		static bool HasComponentFunc(MonoType* type)
		{
			return s_HasComponentsFuncs.find(type) != s_HasComponentsFuncs.end();
		}
		/*
		* Returns based on if Entity has Component type.
		*
		* @param type : Component to get.
		* @param entity : Entity to get Component from.
		*/
		static bool GetHasComponentFunc(MonoType* type, Ref<Scene> scene, Entity entity)
		{
			return s_HasComponentsFuncs.at(type)(scene, entity);
		}

		/*
		* Initializes Mono, registers functions, loads assemblies & classes, then registers components.
		* Sets base Entity class.
		* Should be called after Project initialization.
		*/
		static void Init();
		/*
		* Shuts down Mono
		*/
		static void Shutdown();
		
		/*
		* Reloads Mono Assemblies
		*/
		static void ReloadAssembly();

		/*
		* Returns pointer to C# method
		* @param scriptHandle : Script Asset Handle. Script will be retrieved from s_Data.Scripts
		* @param method :  pair<Name, paramCount>. 
		* Example Names: Constructor(".ctor"), Create("OnCreate"), Update("OnUpdate")
		*/
		static MonoMethod* GetMethod(UUID scriptHandle, const Method& method);

		/*
		* Returns pointer to C# method
		* @param script : Script Asset
		* @param method :  pair<Name, paramCount>. 
		* Example Names: Constructor(".ctor"), Create("OnCreate"), Update("OnUpdate")
		*/
		static MonoMethod* GetMethod(Ref<Script> script, const Method& method);

		/*
		*
		* @param instance : ScriptInstance runtime object. Get by InstantiateScriptObject(Ref<Script>, bool)
		* @param method : method from GetMethod(const std::string&, int)
		* @param params : params required by method
		*/
		static MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params);

		/*
		* Calls InstantiateClass(Ref<Script>, bool), then instantiates & returns runtime script object
		* Runtime Script Object should be assigned to Script Instance Object.
		* @param scriptHandle : handle of script to instantiate.
		* @param isCore : true if script is base Entity Script
		*/
		static MonoObject* InstantiateScriptObject(UUID scriptHandle, bool isCore = false);

		/*
		* Instantiates & sets internal script class
		*
		* @param script : script to instantiate.
		*/
		static void InstantiateScriptClass(Ref<Script> script);

		/*
		* Clears all EntityScriptInstances
		*/
		static void OnStop();

		/*
		* Returns if handled by GE-ScriptCore::Event::OnEvent(Event&).
		* Entity::IDComponent::ID must correspond to one EntityScriptInstances key
		* @param e : Event to handle
		* @param entity : Entity to call scripting event for
		*/
		static bool OnEvent(Event& e, Scene* scene, Entity entity);
		/*
		* Creates ScriptInstance if Entity has assigned ScriptComponent::Class
		* 
		* @param scene : 
		* @param entity : entity with ScriptComponent
		*/
		static void OnCreateScript(Scene* scene, Entity entity);
		/*
		* Updates ScriptInstance if corresponding Entity UUID exists.
		* @param entity : entity with ScriptComponent
		* @param timestep : change in time
		*/
		static void OnUpdateScript(Scene* scene, Entity entity, float timestep);
	private:
		/*
		* Returns new, instantiated Ref<Script>.
		* If Ref<Script> already exists, returns existing
		* @param nameSpace : example "nameSpace"
		* @param className : example "className"
		*/
		static Ref<Script> CreateOrReloadScript(const std::string& nameSpace, const std::string& className);

		/*
		* Will not initialize if Data::CoreDomain already exists.
		* Sets assemblies path = "mono/Lib" & initializes Data::CoreDomain
		*/
		static void InitMono();
		/*
		* Unloads Root & App Domain
		*/
		static void ShutdownMono();

		static MonoDomain* LoadCoreDomain(const std::string& file);
		static MonoDomain* LoadAppDomain(const std::string& file);
		static void ClearCoreDomain(MonoDomain* coreDomain);
		static void ClearAppDomain(MonoDomain* appDomain);
		/*
		* Returns assembly loaded from filePath. Used for Core & App
		* @param assemblyPath : expecting fullPath, including Project
		*/
		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath);
		static MonoImage* LoadMonoImage(MonoAssembly* assembly);
		/*
		* Clears Data::ScriptClasses & sets new Data::ScriptClasses, including class fields, from Core & App.
		*/
		static void LoadAssemblyClasses();
		/*
		* Registers all internal calls
		*/
		static void RegisterFunctions();
		/*
		* If found, adds HasComponentFuncs for T.
		*/
		template<typename T>
		static void RegisterComponent();
		/*
		* Registers all Components using RegisterComponent<T>()
		*/
		static void RegisterComponents();

		/*
		* If file change is "modified", sets Scripting Assemblies to reload next time the main thread is updated.
		* @param path : App Assembly path
		* @param changeType : specific change event  
		*/
		static void OnFileSystemAppAssemblyEvent(const std::string& path, const filewatch::Event changeType);

	private:
		static Data s_Data;

		static std::unordered_map<MonoType*, std::function<bool(Ref<Scene>, Entity)>> s_HasComponentsFuncs;
		static std::unordered_map<std::string, ScriptField::Type> s_ScriptFieldTypeNames;
	};

}