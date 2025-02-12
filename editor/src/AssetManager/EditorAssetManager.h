#pragma once

#include "GE/Asset/Assets/Scene/Components/Components.h"
#include "GE/Asset/AssetManager.h"
#include "GE/Asset/Registry/AssetRegistry.h"
#include "GE/Core/Application/Layer/Layer.h"

namespace GE
{
	class EditorAssetManager : public AssetManager
	{
	public:
		EditorAssetManager(const AssetMap& assetMap = AssetMap());
		~EditorAssetManager() override;

		void InvalidateAssets() override;

		/*
		* Returns nullptr if handle does NOT exist
		* or if asset can not be loaded
		*/
		Ref<Asset> GetAsset(UUID handle) override;
		const AssetMap& GetLoadedAssets() override;

		bool HandleExists(UUID handle) override;
		bool AssetLoaded(UUID handle) override;

		bool AddAsset(Ref<Asset> asset) override;
		bool RemoveAsset(UUID handle) override;

		bool SerializeAssets() override;
		bool DeserializeAssets() override;

		inline Ref<AssetRegistry> GetAssetRegistry() { return m_AssetRegistry; }
		Ref<Asset> GetAsset(const std::filesystem::path& filePath);
		bool AddAsset(const AssetMetadata& metadata);

		const AssetMetadata& GetMetadata(UUID handle);

		static const std::map<Pivot, std::string>& GetPivotStrs() { return s_PivotStrs; }
		static const std::string& PivotToString(const Pivot& pivot)
		{
			if (s_PivotStrs.find(pivot) != s_PivotStrs.end())
				return s_PivotStrs.at(pivot);

			return s_PivotStrs.begin()->second;

		}
		static const Pivot& StringToPivot(const std::string& pivotStr)
		{
			for (const auto& [pivot, str] : s_PivotStrs)
			{
				if (str.compare(pivotStr))
					return pivot;
			}
			GE_WARN("Could not determine Pivot enum from string. \n\tReturning default.");
			return s_PivotStrs.begin()->first;
		}

		static const Layer::Type& EditorAssetManager::GetTypeFromName(const std::string& name)
		{
			for (const auto& [layerType, str] : s_LayerTypeStrs)
			{
				if (str == name)
					return layerType;

			}
			return s_LayerTypeStrs.begin()->first;
		}
		
		static const std::map<CanvasMode, std::string>& GetCanvasModeStrs() { return s_CanvasModeStrs; }
		static const std::string& CanvasModeToStr(const CanvasMode& mode)
		{
			if (s_CanvasModeStrs.find(mode) != s_CanvasModeStrs.end())
				return s_CanvasModeStrs.at(mode);

			return s_CanvasModeStrs.begin()->second;

		}
		static const CanvasMode& StrToCanvasMode(const std::string& modeStr)
		{
			for (const auto& [mode, str] : s_CanvasModeStrs)
			{
				if (str == modeStr)
					return mode;
			}
			GE_WARN("Could not determine CanvasMode enum from string. \n\tReturning default.");
			return s_CanvasModeStrs.begin()->first;
		}

		static const std::map<LayoutMode, std::string>& GetLayoutModeStrs() { return s_LayoutModeStrs; }
		static const std::string& LayoutModeToStr(const LayoutMode& mode)
		{
			if (s_LayoutModeStrs.find(mode) != s_LayoutModeStrs.end())
				return s_LayoutModeStrs.at(mode);

			return s_LayoutModeStrs.at(LayoutMode::Horizontal);

		}
		static const LayoutMode& StrToLayoutMode(const std::string& modeStr)
		{
			for (const auto& [mode, str] : s_LayoutModeStrs)
			{
				if (str == modeStr)
					return mode;
			}
			GE_WARN("Could not determine LayoutMode enum from string. \n\tReturning default LayerMode.");
			return s_LayoutModeStrs.begin()->first;
		}

		static const std::map<SliderDirection, std::string>& GetSliderDirStrs() { return s_SliderDirectionStrs; }
		static const std::string& SliderDirectionToString(const SliderDirection& direction)
		{
			if (s_SliderDirectionStrs.find(direction) != s_SliderDirectionStrs.end())
				return s_SliderDirectionStrs.at(direction);

			GE_WARN("Could not determine Slider direction string from enum. \n\tReturning default.");
			return s_SliderDirectionStrs.begin()->second;

		}
		static const SliderDirection& StringToSliderDirection(const std::string& directionStr)
		{
			for (const auto& [direction, str] : s_SliderDirectionStrs)
			{
				if (str == directionStr)
					return direction;
			}
			GE_WARN("Could not determine Slider direction enum from string. \n\tReturning default.");
			return s_SliderDirectionStrs.begin()->first;
		}
	private:
		bool SerializeAsset(Ref<Asset> asset, const AssetMetadata& metadata);
		Ref<Asset> DeserializeAsset(const AssetMetadata& metadata);

		bool SerializeScene(Ref<Asset> asset, const AssetMetadata& metadata);

		Ref<Asset> DeserializeScene(const AssetMetadata& metadata);
		Ref<Asset> DeserializeTexture2D(const AssetMetadata& metadata);
		Ref<Asset> DeserializeFont(const AssetMetadata& metadata);
		Ref<Asset> DeserializeAudio(const AssetMetadata& metadata);
		Ref<Asset> DeserializeScript(const AssetMetadata& metadata);

	private:
		Ref<AssetRegistry> m_AssetRegistry;
		AssetMap m_LoadedAssets;

		static AssetMetadata s_NullMetadata;

		static std::map<Pivot, std::string> s_PivotStrs;
		static std::map<Layer::Type, std::string> s_LayerTypeStrs;
		static std::map<CanvasMode, std::string> s_CanvasModeStrs;
		static std::map<LayoutMode, std::string> s_LayoutModeStrs;

		static std::map<SliderDirection, std::string> s_SliderDirectionStrs;
	};
}

