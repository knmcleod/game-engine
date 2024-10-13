#pragma once

#include "ScriptingUtils.h"
#include "GE/Asset/Assets/Script/Script.h"

#include "GE/Core/Events/Event.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <FileWatch.h>
#pragma warning(pop)

namespace GE
{
	// Forward declarations
	class Entity;

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

	};

	class Scripting
	{
	public:
		struct Data
		{
			void InstantiateBaseEntity()
			{
				EntityScript = CreateRef<Script>("GE", "Entity");
				Scripting::InstantiateScriptClass(EntityScript, true);
			}

			MonoDomain* RootDomain = nullptr;
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

		/*
		* Returns pointer to Core Assembly Image(GE-ScriptCore)
		*/
		static MonoImage* GetCoreImage() { return s_Data.CoreAssemblyImage; }
		/*
		* Returns pointer to App Assembly Image
		*/
		static MonoImage* GetAppImage() { return s_Data.AppAssemblyImage; }

		/*
		* Returns pointer to base Entity constructor with one param(UUID).
		*/
		static MonoMethod* GetEntityConstructor();
		/*
		* Returns Instance of Script value in Data::ScriptIntances using uuid key.
		* If not found, returns nullptr.
		* @param uuid : corresponds to Entity IDComponent::ID
		*/
		static Ref<ScriptInstance> GetScriptInstance(UUID uuid);
		/*
		* Returns ScriptFieldMap value in Data::EntityScriptFields using uuid key.
		* Key doesn't have to exist.
		* @param uuid : corresponds to Entity IDComponent::ID
		*/
		static ScriptFieldMap& GetEntityFields(UUID uuid);
		/*
		* Returns Script value in Data::Scripts using name key.
		* 
		* @param fullName : example "namespace.className"
		*/
		static Ref<Script> GetScript(const std::string& fullName);
		/*
		* Returns Script value in Data::Scripts using handle.
		*
		* @param handle : Script Asset Handle
		*/
		static Ref<Script> GetScript(UUID handle);

		static const std::unordered_map<std::string, Ref<Script>>& GetScripts() { return s_Data.Scripts; }
		
		static void SetScriptHandle(const std::string& fullName, UUID handle);

		/*
		* Returns true if fullName key is found in Data::Scripts
		* @param fullName : includes namespace, example "namespace.className"
		*/
		static bool ScriptExists(const std::string& fullName);
		/*
		* Returns true if fullName key is found in Data::Scripts
		* @param handle : Script Asset Handle
		*/
		static bool ScriptExists(UUID handle);

		/*
		* Returns pointer to C# method
		* @param scriptHandle : Script Asset Handle. Script will be retrieved from s_Data.Scripts
		* @param name : method name, i.e. Constructor(".ctor"), Create("OnCreate"), Update("OnUpdate")
		* @param parameterCount : number of parameters the method takes.
		*/
		static MonoMethod* GetMethod(UUID scriptHandle, const std::string& name, int parameterCount);

		/*
		* Returns pointer to C# method
		* @param script : Script Asset
		* @param name : method name, i.e. Constructor(".ctor"), Create("OnCreate"), Update("OnUpdate")
		* @param parameterCount : number of parameters the method takes.
		*/
		static MonoMethod* GetMethod(Ref<Script> script, const std::string& name, int parameterCount);
		/*
		*
		* @param instance : ScriptInstance runtime object. Get by InstantiateScriptObject(Ref<Script>, bool)
		* @param method : method from GetMethod(const std::string&, int)
		* @param params : params required by method
		*/
		static MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params);

		/*
		* Returns true if HasComponentFunc is found for type.
		* 
		* @param type : Any internal Component type
		*/
		static bool HasComponentFunc(MonoType* type);
		/*
		* Returns based on if Entity has Component type.
		* 
		* @param type : Component to get.
		* @param entity : Entity to get Component from.
		*/
		static bool GetHasComponentFunc(MonoType* type, Entity entity);

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
		* @param isCore : true if script is base Entity Script
		*/
		static void InstantiateScriptClass(Ref<Script> script, bool isCore = false);
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
		* Clears all EntityScriptInstances
		*/
		static void OnStop();
		/*
		* Returns if handled by GE-ScriptCore::Event::OnEvent(Event&).
		* Entity::IDComponent::ID must correspond to one EntityScriptInstances key
		* @param e : Event to handle
		* @param entity : Entity to call scripting event for
		*/
		static bool OnEvent(Event& e, Entity entity);
		/*
		* Creates ScriptInstance if Entity has assigned ScriptComponent::Class
		* @param entity : entity with ScriptComponent
		*/
		static void OnCreateScript(Entity entity);
		/*
		* Updates ScriptInstance if corresponding Entity UUID exists.
		* @param entity : entity with ScriptComponent
		* @param timestep : change in time
		*/
		static void OnUpdateScript(Entity entity, float timestep);
	private:
		/*
		* Will not initialize if Data::RootDomain already exists.
		* Sets assemblies path = "mono/Lib" & initializes Data::RootDomain
		*/
		static void InitMono();
		/*
		* Unloads Root & App Domain
		*/
		static void ShutdownMono();
		/*
		* Returns assembly loaded from filePath. Used for Core & App
		* @param assemblyPath : expecting fullPath, including Project
		*/
		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath);
		/*
		* Loads MonoAssembly into Data::AppAssembly & sets Data::AppAssemblyImage. Initializes FileWater for reloading.
		* @param filePath : app/client filePath
		*/
		static void LoadAppAssembly(const std::filesystem::path& filePath);
		/*
		* Loads MonoAssembly into Data::CoreAssembly & sets Data::CoreAssemblyImage.
		* @param filePath : core filePath
		*/
		static void LoadCoreAssembly(const std::filesystem::path& filePath);
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

		static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_HasComponentsFuncs;
		static std::unordered_map<std::string, ScriptField::Type> s_ScriptFieldTypeNames;
	};

}