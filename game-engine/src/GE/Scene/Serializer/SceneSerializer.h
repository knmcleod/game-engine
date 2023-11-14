#pragma once

#include "GE/Scene/Scene.h"

namespace GE
{
	class SceneSerializer
	{
	private:
		Ref<Scene> m_Scene;
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void SerializeText(const std::string& filePath);
		void SerializeBinary(const std::string& filePath);

		bool DeserializeText(const std::string& filePath);
		bool DeserializeBinary(const std::string& filePath);
	};
}
