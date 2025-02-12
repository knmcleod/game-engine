#include "GE/GEpch.h"

#include "Script.h"

#include "GE/Scripting/Scripting.h"

namespace GE
{
	Script::Script() : Asset(UUID(), Asset::Type::Script)
	{
	}
	Script::Script(const std::string& classNamespace, const std::string& className) : Script()
	{
		m_ClassNamespace = classNamespace;
		m_ClassName = className;
	}

	Script::~Script()
	{
		Invalidate();
	}

	void Script::Invalidate()
	{
		m_Class = nullptr;

		for (auto& [name, field] : m_Fields)
		{
			field.Invalidate();
		}
	}

}