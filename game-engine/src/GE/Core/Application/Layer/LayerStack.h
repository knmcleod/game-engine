#pragma once

#include "Layer.h"

#include <vector>

namespace GE
{
	class LayerStack
	{
		friend class Application;
	public:
		LayerStack() = default;
		virtual ~LayerStack();

		// Inserts Layer at next Index & sets Layer ID
		virtual bool InsertLayer(Ref<Layer> layer);
		// Sets Layers[layerID] = nullptr && layerID = 0
		virtual bool RemoveLayer(Ref<Layer> layer);

		/*
		* Returns Layer at index if exists, 
		*	otherwise returns nullptr 
		* @param index : index in stack
		*/
		Ref<Layer> GetLayerAtIndex(uint64_t index) const
		{ 
			if (index >= p_Layers.size())
				return nullptr;
			return p_Layers.at(index);
		}

		bool LayerExists(uint64_t index)
		{
			for (auto& layer : p_Layers)
			{
				if (layer->GetID() == index)
					return true;
			}
			return false;
		}

	protected:
		std::vector<Ref<Layer>>::iterator begin() { return p_Layers.begin(); }
		std::vector<Ref<Layer>>::iterator end() { return p_Layers.end(); }
	protected:
		std::vector<Ref<Layer>> p_Layers = std::vector<Ref<Layer>>();
	};
}