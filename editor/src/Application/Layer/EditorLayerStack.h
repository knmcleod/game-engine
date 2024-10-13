#pragma once

#include <GE/Core/Application/Layer/LayerStack.h>

namespace GE
{
	class EditorLayerStack : public LayerStack
	{
		friend class Application;
	public:
		EditorLayerStack() = default;
		~EditorLayerStack() override;

		bool InsertLayer(Ref<Layer> layer) override;
		bool RemoveLayer(Ref<Layer> layer) override;

		const std::string GetLayerName(uint64_t index)
		{
			if (index >= m_LayerNames.size())
				return std::string();
			return m_LayerNames.at(index);
		}
		uint64_t GetLayerIndex(const std::string& name)
		{
			uint64_t layerIndex = 0;
			for (auto& layerName : m_LayerNames)
			{
				if (layerName == name)
					break;
				layerIndex++;
			}

			return layerIndex;
		}

		bool LayerNameExists(const std::string& layerName)
		{
			for (auto& name : m_LayerNames)
			{
				if (name == layerName)
					return true;
			}
			return false;
		}
		
		bool InsertLayer(Ref<Layer> layer, const std::string& name);

		// Inserts name at next Index
		bool InsertLayerName(const std::string& name);
		bool PopLayerName(uint64_t index);
	private:
		// Indices matches LayerStack::p_Layers
		std::vector<std::string> m_LayerNames = std::vector<std::string>();
	};
}