#include "GE/GEpch.h"

#include "EditorLayerStack.h"

#include "EditorLayer.h"

namespace GE
{
	EditorLayerStack::~EditorLayerStack()
	{
		m_LayerNames.clear();
		m_LayerNames = std::vector<std::string>();
	}

	bool EditorLayerStack::InsertLayer(Ref<Layer> layer)
	{
		std::string newLayerName = std::string("NewLayerName");
		return InsertLayer(layer, newLayerName);
	}

	bool EditorLayerStack::RemoveLayer(Ref<Layer> layer)
	{
		return false;
	}

	bool EditorLayerStack::InsertLayer(Ref<Layer> layer, const std::string& name)
	{
		if (LayerNameExists(name) || !LayerStack::InsertLayer(layer))
			return false;
		if (!InsertLayerName(name))
				GE_WARN("Could not add Layer Name, despite Layer being added.");
		return true;
	}

	bool EditorLayerStack::InsertLayerName(const std::string& name)
	{
		if (LayerNameExists(name))
			return false;
		m_LayerNames.insert(m_LayerNames.begin() + m_LayerNames.size(), name);
		return true;
	}

	bool EditorLayerStack::PopLayerName(uint64_t index)
	{
		if (index >= m_LayerNames.size())
			return false;
		std::vector<std::string> temp = std::vector<std::string>();
		for (uint64_t i = 0; i < m_LayerNames.size() - 1; i++)
		{
			if (i == index)
				continue;
			temp.push_back(m_LayerNames.at(i));
		}

		m_LayerNames.clear();
		m_LayerNames = temp;
		temp.clear();
		return true;
	}
}