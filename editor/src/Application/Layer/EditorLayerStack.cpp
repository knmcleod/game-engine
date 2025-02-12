#include "GE/GEpch.h"

#include "EditorLayerStack.h"

#include "EditorLayer.h"

namespace GE
{
	EditorLayerStack::~EditorLayerStack()
	{
		m_LayerNames.clear();
		m_LayerNames = std::map<uint64_t, std::string>();
	}

	bool EditorLayerStack::AddLayer(Ref<Layer> layer)
	{
		std::string newLayerName = std::string("NewLayer");
		return AddLayer(layer, newLayerName);
	}

	bool EditorLayerStack::InsertLayer(Ref<Layer> layer)
	{
		std::string newLayerName = std::string("NewLayer");
		return InsertLayer(layer, newLayerName);
	}

	bool EditorLayerStack::RemoveLayer(const uint64_t& id)
	{
		if (LayerStack::RemoveLayer(id))
		{
			m_LayerNames.erase(id);
			return true;
		}
		return false;
	}

	Ref<Layer> EditorLayerStack::GetLayer(const uint64_t& id)
	{
		if (!LayerExists(id))
			return nullptr;
		return p_Layers.at(id);
	}

	bool EditorLayerStack::AddLayer(Ref<Layer> layer, const std::string& name)
	{
		if (!AddLayerName(name))
			GE_WARN("EditorLayerStack::AddLayer(Ref<Layer>, const std::string&) - \n\tCould not add Layer Name.");
		return LayerStack::AddLayer(layer);
	}

	bool EditorLayerStack::InsertLayer(Ref<Layer> layer, const std::string& name)
	{
		if (!layer || name == std::string())
			return false;
		if (!InsertLayerName(layer->GetID(), name))
			GE_WARN("EditorLayerStack::InsertLayer(Ref<Layer>, const std::string&) - \n\tCould not insert Layer Name.");
		return LayerStack::InsertLayer(layer);
	}

	bool EditorLayerStack::AddLayerName(const std::string& name)
	{
		if (LayerNameExists(name))
			return false;
		m_LayerNames.insert({ (uint64_t)m_LayerNames.size(), name });
		return true;
	}

	bool EditorLayerStack::InsertLayerName(const uint64_t& id, const std::string& name)
	{
		if (LayerNameExists(name))
		{
			GE_ERROR("EditorLayerStack::InsertLayerName(const uint64_t&, const std::string&) - \n\t Could not insert Layer Name. Name already exists.");
			return false;
		}
		if (LayerExists(id))
		{
			// temp copy to edit then set main layers
			std::map<uint64_t, std::string> temp = std::map<uint64_t, std::string>();

			// starting at layerID before, emplace all LayerIDs until beginning
			for (auto& it = m_LayerNames.find(id)--; it != m_LayerNames.begin(); )
			{
				temp.emplace(it->first, it->second);
				it--;
			}

			// New layerName at layerID
			temp.emplace(id, name);

			// Get old layerName at layerID
			// emplace old layerNames at new layerIDs until end
			for (auto& it = m_LayerNames.find(id); it != m_LayerNames.end(); )
			{
				uint64_t newLayerID = temp.size();
				temp.emplace(newLayerID, it->second); // Add old

				it++;
			}
	
			// Set
			m_LayerNames.clear();
			m_LayerNames = std::map<uint64_t, std::string>(temp);
		}
		else
		{
			m_LayerNames.insert({ id, name });
		}
		return true;

	}

	bool EditorLayerStack::PopLayerName(const uint64_t& id)
	{
		if (id >= m_LayerNames.size())
			return false;
		m_LayerNames.erase(id);
		return true;
	}
}