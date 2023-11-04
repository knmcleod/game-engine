#pragma once

#include <GE/GE.h>

#include <glm/gtc/type_ptr.hpp>

namespace GE
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer(const std::string& name = "EditorLayer");
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep timestep) override;
		virtual void OnEvent(Event& e) override;
		virtual void OnImGuiRender() override;
		virtual void OnWindowResize(WindowResizeEvent& e) override;

	private:
		OrthographicCameraController m_OrthoCameraController;

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

		// Temp
		glm::vec4 m_ShaderColor = { 0.2f, 0.3f, 0.8f, 1.0f };
		Ref<Texture2D> m_SpriteSheet;
		Ref<SubTexture2D> m_Sprite;

		Ref<Framebuffer> m_Framebuffer;

	};
}
