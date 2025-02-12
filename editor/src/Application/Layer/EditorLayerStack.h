#pragma once

#include <GE/Core/Application/Layer/LayerStack.h>

namespace GE
{
	class EditorLayerStack : public LayerStack
	{
		friend class EditorApplication;
		friend class EditorAssetManager;
	public:
		EditorLayerStack() = default;
		~EditorLayerStack() override;

		bool AddLayer(Ref<Layer> layer) override;
		bool InsertLayer(Ref<Layer> layer) override;
		bool RemoveLayer(const uint64_t& id) override;

		const std::string& GetLayerName(uint64_t index)
		{
			if (index >= m_LayerNames.size())
				return defaultStr;
			return m_LayerNames.at(index);
		}
		uint64_t GetLayerIndex(const std::string& name)
		{
			uint64_t layerIndex = 0;
			for (auto& [id, layerName] : m_LayerNames)
			{
				if (layerName == name)
					break;
				layerIndex++;
			}
			return layerIndex;
		}

		bool LayerNameExists(const std::string& name)
		{
			for (auto& [id, layerName] : m_LayerNames)
			{
				if (layerName == name)
					return true;
			}
			return false;
		}
		
		Ref<Layer> GetLayer(const uint64_t& id);
		/*
		* Returns true if layer & name don't exist & are added
		* 
		* @param layer : layer to add
		* @param name : Layer name 
		*/
		bool AddLayer(Ref<Layer> layer, const std::string& name);
		/*
		* Returns true if layer & name are added.
		* Will resize/move layers & names to insert at id
		* 
		* @param layer : layer to add
		* @param name : Layer Name
		*/
		bool InsertLayer(Ref<Layer> layer, const std::string& name);

		/*
		* Returns true if name doesn't exist & can be added
		* 
		* @param name : Layer Name
		*/
		bool AddLayerName(const std::string& name);
		/*
		* Returns true if name can be added
		* Will resize/move to insert name at id
		* 
		* @param id : Layer::ID
		* @param : Layer Name
		*/
		bool InsertLayerName(const uint64_t& id, const std::string& name);
		/*
		* Returns true if id exists & is removed
		* 
		* @param id : Layer::ID to remove
		*/
		bool PopLayerName(const uint64_t& id);
	private:
		// Key value matches LayerStack::Layers Key
		std::map<uint64_t, std::string> m_LayerNames = std::map<uint64_t, std::string>();
		std::string defaultStr = std::string();
	};
}