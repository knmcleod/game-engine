#pragma once

#include "Layer.h"

#include <map>

namespace GE
{
	class LayerStack
	{
		friend class Application;
		using Layers = std::map<uint64_t, Ref<Layer>>;
	public:
		LayerStack();
		virtual ~LayerStack();

		bool LayerExists(uint64_t index)
		{
			return p_Layers.find(index) != p_Layers.end();
		}

		/* 
		* Inserts Layer at next Index & sets Layer::ID
		*/
		virtual bool AddLayer(Ref<Layer> layer);
		/*
		* Inserts Layer at Layer::ID. 
		* Resizes & moves preexisting Layers as needed.
		* Layer::ID will be updated
		*/
		virtual bool InsertLayer(Ref<Layer> layer);
		/*
		* Returns true if id is found & removed.
		* Will set Layer::ID = 0.
		*/
		virtual bool RemoveLayer(const uint64_t& id);

	protected:
		/*
		* Returns Layer at index, even if index isn't initialized.
		* Can return nullptr
		* @param index : position
		*/
		Ref<Layer> GetLayerAtIndex(uint64_t index)
		{
			return p_Layers[index];
		}

		Layers::iterator begin() { return p_Layers.begin(); }
		Layers::iterator end() { return p_Layers.end(); }
	protected:
		Layers p_Layers = Layers();
	};
}