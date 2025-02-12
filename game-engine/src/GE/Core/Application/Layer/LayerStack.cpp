#include "GE/GEpch.h"

#include "LayerStack.h"

namespace GE
{
	LayerStack::LayerStack()
	{

	}
	LayerStack::~LayerStack()
	{
		for (auto& [id, layer] : p_Layers)
		{
			layer->OnDetach();
		}
		p_Layers.clear();
		p_Layers = Layers();
	}

	bool LayerStack::AddLayer(Ref<Layer> layer)
	{
		uint64_t nextIndex = p_Layers.size();
		if (LayerExists(nextIndex))
			return false;
		p_Layers.insert({ nextIndex, layer });
		layer->SetID(nextIndex);
		return true;
	}

	bool LayerStack::InsertLayer(Ref<Layer> layer)
	{
		uint64_t layerID = layer->GetID();
		if (LayerExists(layerID))
		{
			// Layer that exists at ID is same layer, no need to update
			if (p_Layers.at(layerID) == layer)
				return false;

			// temp copy to edit then set main layers
			Layers temp = Layers();

			// starting at layerID before, emplace all layers until beginning
			for (auto& it = p_Layers.find(layerID)--; it != p_Layers.begin(); )
			{
				temp.emplace(it->first, it->second);

				it--;
			}

			// New layer at layerID
			temp.emplace(layerID, layer);

			// Get old layer at layerID
			// emplace old layers at new layerIDs & update Layer::ID
			for (auto& it = p_Layers.find(layerID); it != p_Layers.end(); )
			{
				uint64_t newLayerID = temp.size();
				it->second->SetID(newLayerID);
				temp.emplace(newLayerID, it->second); // Add old

				it++;
			}

			// Set
			p_Layers.clear();
			p_Layers = Layers(temp);
		}
		else
		{
			p_Layers.insert({ layerID, layer });
		}
		return true;
	}

	bool LayerStack::RemoveLayer(const uint64_t& id)
	{
		if (p_Layers.find(id) != p_Layers.end())
		{
			p_Layers.at(id)->ClearID();
			p_Layers.erase(id);
			return true;
		}
		return false;
	}

}