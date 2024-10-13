#include "GE/GEpch.h"

#include "LayerStack.h"

namespace GE
{
	LayerStack::~LayerStack()
	{
		for (Ref<Layer> layer : p_Layers)
		{
			layer->OnDetach();
		}
		p_Layers.clear();
		p_Layers = std::vector<Ref<Layer>>();
	}

	bool LayerStack::InsertLayer(Ref<Layer> layer)
	{
		uint64_t nextIndex = p_Layers.size();
		if (LayerExists(nextIndex))
			return false;
		p_Layers.insert(p_Layers.begin() + nextIndex, layer);
		layer->SetID(nextIndex);
		return true;
	}

	bool LayerStack::RemoveLayer(Ref<Layer> layer)
	{
		auto it = std::find(p_Layers.begin(), p_Layers.end(), layer);
		if (it != p_Layers.end())
		{
			p_Layers.at(layer->GetID()) = nullptr;
			layer->ClearID();
			return true;
		}
		return false;
	}

}