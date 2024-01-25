#pragma once

#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>

#include <filesystem>
#include <string>

extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
}

namespace GE
{
	class Scripting
	{
	public:
		static void Init();
		static void Shutdown();
	private:
		static void InitMono();
		static void ShutdownMono();

		static void LoadAssembly(const std::filesystem::path& filepath);
		static MonoObject* InstantiateClass(MonoClass* monoClass);

		static char* ReadBytes(const std::filesystem::path& filePath, uint32_t* fileSize);
		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath);

		friend class ScriptClass;
	};

	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className);

		MonoObject* Instantiate();
		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);
	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		MonoClass* m_MonoClass = nullptr;
	};

	class ScriptGlue
	{
	public:
		static void RegisterFunctions();
	};
}