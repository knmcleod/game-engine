#include "GE/GEpch.h"

#include "GUILayer.h"
#include "GE/Asset/Assets/Scene/Scene.h"

#include "GE/Core/Application/Application.h"

#include "GE/Project/Project.h"
#include "GE/Rendering/Renderer/Renderer.h"

#include "GE/Scripting/Scripting.h"

namespace GE
{	
	float GUILayer::s_ReactionDelay = 60.0f;

	GUILayer::GUILayer(uint64_t id) : Layer(id)
	{

	}
	GUILayer::~GUILayer() 
	{
	}

	void GUILayer::RenderEntity(Ref<Scene> scene, Entity entity, glm::vec3& translationOffset, glm::vec3& rotationOffset)
	{
		GE_PROFILE_FUNCTION();
		if (!scene || !entity)
			return;

		auto& rc = scene->GetComponent<RenderComponent>(entity);
		auto& ac = scene->GetComponent<ActiveComponent>(entity);
		if (rc.Rendered || (!ac.Active || ac.Hidden) || !rc.IDHandled(p_Config.ID))
			return; // Entity; 1: Shouldn't be rendered. 2: Should be active && visible. 3: Should be handled by GUILayer::ID

		auto& trsc = scene->GetComponent<TransformComponent>(entity);
		
		// First, render self/parent
		if (scene->HasComponent<GUIComponent>(entity))
		{
			auto& guiC = scene->GetComponent<GUIComponent>(entity);

			if (scene->HasComponent<GUICanvasComponent>(entity))
			{
				rc.Rendered = true;
			}
			// TODO : GUIMaskComponent: Render Child before Parent

			if (scene->HasComponent<GUIImageComponent>(entity))
			{
				auto& guiIC = scene->GetComponent<GUIImageComponent>(entity);

				Renderer::Draw(trsc.GetTransform(translationOffset, rotationOffset), trsc.GetPivot(), guiIC, entity);

				rc.Rendered = true;
			}

			if (scene->HasComponent<GUIButtonComponent>(entity))
			{
				auto& guiBC = scene->GetComponent<GUIButtonComponent>(entity);

				Renderer::Draw(trsc.GetTransform(translationOffset, rotationOffset), trsc.GetPivot(), guiBC, guiC.CurrentState, entity);

				rc.Rendered = true;

			}

			if (scene->HasComponent<GUIInputFieldComponent>(entity))
			{
				auto& guiIFC = scene->GetComponent<GUIInputFieldComponent>(entity);

				if (guiIFC.FillBackground)
					guiIFC.TextSize = Renderer::GetFontTextSize(guiIFC);
				
				Renderer::Draw(trsc.GetTransform(translationOffset, rotationOffset), trsc.GetPivot(), guiIFC, guiC.CurrentState, entity);

				rc.Rendered = true;

			}

			if (scene->HasComponent<GUISliderComponent>(entity))
			{
				auto& guiSC = scene->GetComponent<GUISliderComponent>(entity);

				Renderer::Draw(trsc.GetTransform(translationOffset, rotationOffset), trsc.GetPivot(), guiSC, guiC.CurrentState, entity);

				rc.Rendered = true;

			}

			if (scene->HasComponent<GUICheckboxComponent>(entity))
			{
				auto& guiCB = scene->GetComponent<GUICheckboxComponent>(entity);

				Renderer::Draw(trsc.GetTransform(translationOffset, rotationOffset), trsc.GetPivot(), guiCB, guiC.CurrentState, entity);

				rc.Rendered = true;

			}

			// TODO : GUIScrollRectComponent && GUIScrollbarComponent

		}

		// Then, render children offset from self/parent
		auto& rsc = scene->GetComponent<RelationshipComponent>(entity);
		if (!rsc.GetChildren().empty())
		{
			translationOffset += trsc.Translation;
			rotationOffset += trsc.Rotation;
			if (scene->HasComponent<GUILayoutComponent>(entity))
			{
				auto& guiLOC = scene->GetComponent<GUILayoutComponent>(entity);
				glm::vec2 layoutOffset = guiLOC.StartingOffset;
				for (const UUID& childID : rsc.GetChildren())
				{
					Entity childEntity = scene->GetEntityByUUID(childID);
					auto& childTRSC = scene->GetComponent<TransformComponent>(childEntity);

					childTRSC.Translation = glm::vec3(layoutOffset, 0.0f);
					childTRSC.Scale = glm::vec3(guiLOC.ChildSize, 1.0f);

					RenderEntity(scene, childEntity, translationOffset, rotationOffset);
					layoutOffset += guiLOC.GetEntityOffset();
				}
			}
			else
			{
				for (const UUID& childID : rsc.GetChildren())
				{
					Entity childEntity = scene->GetEntityByUUID(childID);
					RenderEntity(scene, childEntity, translationOffset, rotationOffset);
				}
			}
		}
	}

	void GUILayer::OnAttach(Ref<Scene> scene)
	{
		Focus(scene, scene->GetGUICanvasEntity(p_Config.ID));
	}

	void GUILayer::OnUpdate(Ref<Scene> scene, Timestep ts)
	{
		Layer::OnUpdate(scene, ts);

		// TODO : Handle GUIState::Active
		// Timeout
		if (m_Steps >= s_ReactionDelay)
		{
			if (m_NavData.SelectedEntity && scene->HasComponent<GUIButtonComponent>(m_NavData.SelectedEntity))
			{
				if (m_NavData.SelectedEntity == m_NavData.FocusedEntity)
				{
					Unselect(scene);
					Focus(scene, m_NavData.FocusedEntity);
				}
				else
					Unselect(scene);
			}

			m_Steps = 0;
		}
		m_Steps++;

		if (Entity newHovered = Application::GetHoveredEntity()) // New hovered. Unhover old if exists & Hover new
			Hover(scene, newHovered);
		else if (m_NavData.HoveredEntity) // No new hovered. Unhover & clear old.
			Unhover(scene);

		if (m_NavData.HoveredEntity && Application::IsMousePressed(Input::MOUSE_BUTTON_1) && scene->HasComponent<GUISliderComponent>(m_NavData.HoveredEntity))
		{
			auto& guiSC = scene->GetComponent<GUISliderComponent>(m_NavData.HoveredEntity);

			float halfWidth = 0.0f;
			float halfHeight = 0.0f;
			if (Ref<Framebuffer> fb = Application::GetFramebuffer())
			{
				halfWidth = (float)fb->GetWidth() / 2.0f;
				halfHeight = (float)fb->GetHeight() / 2.0f;
			}
			const glm::vec2 fbCursor = Application::GetFramebufferCursor();

			// Offset fbCursor by fb halfWidth & halfHeight
			// fbCursors origin is lower left corner, while camera origin is center.
			// entities are offset based on camera origin, not fb origin
			glm::vec2 offsetCursor = glm::vec2(fbCursor.x - halfWidth, halfHeight - fbCursor.y);

			float aspectRatio = 1.0f;
			if (Entity pcEntity = scene->GetPrimaryCameraEntity(p_Config.ID))
			{
				auto& cc = scene->GetComponent<CameraComponent>(pcEntity);
				aspectRatio = cc.ActiveCamera.GetAspectRatio();
			}

			// scale to fit
			offsetCursor /= aspectRatio * 100.0f;

			auto& trsc = scene->GetComponent<TransformComponent>(m_NavData.HoveredEntity);
			const std::pair<glm::vec2, glm::vec2> entityBounds = trsc.GetBounds();

			glm::vec3 translationOffset = glm::vec3(0.0f);
			glm::vec3 rotationOffset = glm::vec3(0.0f);
			scene->GetTotalOffset(m_NavData.HoveredEntity, translationOffset, rotationOffset);

			float totalEntityWidth = (entityBounds.second.x - entityBounds.first.x);
			float worldSpaceToEntityStartWidth = translationOffset.x;
			float worldSpaceToEntityEndWidth = worldSpaceToEntityStartWidth + totalEntityWidth;

			float totalEntityHeight = (entityBounds.second.y - entityBounds.first.y);
			float worldSpaceToEntityStartHeight = translationOffset.y;
			float worldSpaceToEntityEndHeight = worldSpaceToEntityStartHeight + totalEntityHeight;

			switch (guiSC.Direction)
			{
			case SliderDirection::Left:
			{
				if (offsetCursor.x >= worldSpaceToEntityStartWidth && offsetCursor.x <= worldSpaceToEntityEndWidth)
					guiSC.Fill = worldSpaceToEntityEndWidth - offsetCursor.x;
			}
			break;
			case SliderDirection::Center:
			{
				if (offsetCursor.x >= worldSpaceToEntityStartWidth && offsetCursor.x <= worldSpaceToEntityEndWidth)
				{
					float worldSpaceToEntityCenter = worldSpaceToEntityEndWidth - (totalEntityWidth / 2.0f);
					if (offsetCursor.x >= worldSpaceToEntityCenter)
						guiSC.Fill = offsetCursor.x - worldSpaceToEntityStartWidth;
					else
						guiSC.Fill = worldSpaceToEntityEndWidth - offsetCursor.x;
				}
			}
			break;
			case SliderDirection::Right:
			{
				if (offsetCursor.x >= worldSpaceToEntityStartWidth && offsetCursor.x <= worldSpaceToEntityEndWidth)
					guiSC.Fill = offsetCursor.x - worldSpaceToEntityStartWidth;
			}
			break;
			case SliderDirection::Top:
			{
				if (offsetCursor.y >= worldSpaceToEntityStartHeight && offsetCursor.y <= worldSpaceToEntityEndHeight)
					guiSC.Fill = 1.0f - (offsetCursor.y - worldSpaceToEntityStartHeight);
			}
			break;
			case SliderDirection::Middle:
			{
				if (offsetCursor.y >= worldSpaceToEntityStartHeight && offsetCursor.y <= worldSpaceToEntityEndHeight)
				{
					float worldSpaceToEntityCenter = worldSpaceToEntityEndHeight - (totalEntityHeight / 2.0f);
					if (offsetCursor.y >= worldSpaceToEntityCenter)
						guiSC.Fill = offsetCursor.y - worldSpaceToEntityStartHeight;
					else
						guiSC.Fill = worldSpaceToEntityEndHeight - offsetCursor.y;
				}
			}
			break;
			case SliderDirection::Bottom:
			{
				if (offsetCursor.y >= worldSpaceToEntityStartHeight && offsetCursor.y <= worldSpaceToEntityEndHeight)
					guiSC.Fill = 1.0f - (worldSpaceToEntityEndHeight - offsetCursor.y);
			}
			break;

			default:
				GE_CORE_WARN("Unknown SliderDirection");
				break;
			}
		}

	}

	void GUILayer::OnRender(Ref<Scene> scene, const Camera*& camera)
	{
		if (scene && camera)
		{
			Renderer::Open(camera);
			std::vector<Entity> entities = scene->GetAllEntitiesWith<GUICanvasComponent>();
			for (Entity entity : entities)
			{
				auto& idc = scene->GetComponent<IDComponent>(entity);
				auto& ac = scene->GetComponent<ActiveComponent>(entity);
				auto& rsc = scene->GetComponent<RelationshipComponent>(entity);
				auto& rc = scene->GetComponent<RenderComponent>(entity);
				if (idc.ID == rsc.GetParent() && rc.IDHandled(p_Config.ID)) // Is Parent Canvas & can be rendered by GUILayer
				{
					auto& trsc = scene->GetComponent<TransformComponent>(entity);
					auto& guiCC = scene->GetComponent<GUICanvasComponent>(entity);
					
					if(guiCC.ControlMouse)
						Application::SetCursorMode(guiCC.ShowMouse ? Input::CursorMode::Normal : Input::CursorMode::Disabled);

					switch (guiCC.Mode)
					{
					case CanvasMode::Overlay: // GUI Components follow Active Camera
					{
						trsc.Translation = camera->GetPosition();
					}
					break;
					case CanvasMode::World: // GUI Components exist in worldspace like any other Component
						break;
					}

					RenderEntity(scene, entity, glm::vec3(0.0f), glm::vec3(0.0f));
				}
			}

			entities.clear();
			entities = std::vector<Entity>();

			Renderer::Close();
		}
	}
 
	void GUILayer::TraverseGUIEntity(KeyPressedEvent& e, Ref<Scene> scene, const Entity& entity)
	{
		if (!scene || !entity || !scene->HasComponent<IDComponent>(entity) 
				|| !scene->HasComponent<RelationshipComponent>(entity) 
				|| !scene->HasComponent<GUIComponent>(entity))
			return;

		auto& idc = scene->GetComponent<IDComponent>(entity);
		auto& rsc = scene->GetComponent<RelationshipComponent>(entity);
		auto& guiC = scene->GetComponent<GUIComponent>(entity);

		Entity nextEntity = Entity();

		if (guiC.IsNavigatable)
		{
			if (scene->HasComponent<GUILayoutComponent>(entity))
			{
				auto& guiLOC = scene->GetComponent<GUILayoutComponent>(entity);
				m_NavData.IsVertical = guiLOC.Mode == LayoutMode::Vertical;
				m_NavData.IsLayout = true;
			}
			else
			{
				m_NavData.IsVertical = e.GetKeyCode() == Input::KEY_UP || e.GetKeyCode() == Input::KEY_DOWN;
				m_NavData.IsLayout = false;
			}

			switch (e.GetKeyCode())
			{
			case Input::KEY_ESCAPE:
				Unfocus(scene);
				break;
			case Input::KEY_UP:
			{
				if (m_NavData.IsVertical)
				{
					if (m_NavData.IsLayout || rsc.GetParent() == idc.ID) // Canvas/Root Entity
					{
						nextEntity = scene->GetEntityByUUID(rsc.GetBack());
					}
					else
					{
						if (Entity parentEntity = scene->GetEntityByUUID(rsc.GetParent()))
						{
							auto& parentRSC = scene->GetComponent<RelationshipComponent>(parentEntity);
							nextEntity = scene->GetEntityByUUID(parentRSC.GetPrevious(idc.ID));
						}
					}
				}
				else
				{
					if (rsc.GetParent() == idc.ID) // Canvas/Root Entity
					{
						std::vector<Entity> entities = scene->GetAllEntitiesWith<GUIComponent>();

						bool next = false;
						for (const auto& guiEntity : entities)
						{
							auto& nextIDC = scene->GetComponent<IDComponent>(guiEntity);
							if (next && scene->GetComponent<RelationshipComponent>(guiEntity).GetParent() == nextIDC.ID)
							{
								nextEntity = guiEntity;
								break;
							}

							if (nextIDC.ID == idc.ID)
								next = true;
						}
						entities.clear();
						entities = std::vector<Entity>();
					}
					else
					{
						nextEntity = scene->GetEntityByUUID(rsc.GetParent());
					}
				}
			}
			break;
			case Input::KEY_DOWN:
			{
				if (m_NavData.IsVertical)
				{
					if (m_NavData.IsLayout || rsc.GetParent() == idc.ID) // Canvas/Root Entity
					{
						nextEntity = scene->GetEntityByUUID(rsc.GetFront());
					}
					else
					{
						if (Entity parentEntity = scene->GetEntityByUUID(rsc.GetParent()))
						{
							auto& parentRSC = scene->GetComponent<RelationshipComponent>(parentEntity);
							nextEntity = scene->GetEntityByUUID(parentRSC.GetNext(idc.ID));
						}
					}
				}
				else
				{
					if (rsc.GetParent() == idc.ID) // Canvas/Root Entity
					{
						std::vector<Entity> entities = scene->GetAllEntitiesWith<GUIComponent>();

						bool next = false;
						for (const auto& guiEntity : entities)
						{
							auto& nextIDC = scene->GetComponent<IDComponent>(guiEntity);
							if (next && scene->GetComponent<RelationshipComponent>(guiEntity).GetParent() == nextIDC.ID)
							{
								nextEntity = guiEntity;
								break;
							}

							if (nextIDC.ID == idc.ID)
								next = true;
						}
						entities.clear();
						entities = std::vector<Entity>();
					}
					else
					{
						nextEntity = scene->GetEntityByUUID(rsc.GetParent());
					}
				}
			}
			break;
			case Input::KEY_LEFT:
			{
				if (!m_NavData.IsVertical && m_NavData.IsLayout)
				{
					if (rsc.GetParent() == idc.ID) // Canvas/Root Entity
					{
						nextEntity = scene->GetEntityByUUID(rsc.GetBack());
					}
					else
					{
						if (Entity parentEntity = scene->GetEntityByUUID(rsc.GetParent()))
						{
							auto& parentRSC = scene->GetComponent<RelationshipComponent>(parentEntity);
							nextEntity = scene->GetEntityByUUID(parentRSC.GetPrevious(idc.ID));
						}
					}
				}
				else
				{
					if (rsc.GetParent() == idc.ID) // Canvas/Root Entity
					{
						std::vector<Entity> entities = scene->GetAllEntitiesWith<GUIComponent>();

						bool next = false;
						for (const auto& guiEntity : entities)
						{
							auto& nextIDC = scene->GetComponent<IDComponent>(guiEntity);
							if (next && scene->GetComponent<RelationshipComponent>(guiEntity).GetParent() == nextIDC.ID)
							{
								nextEntity = guiEntity;
								break;
							}

							if (nextIDC.ID == idc.ID)
								next = true;
						}
						entities.clear();
						entities = std::vector<Entity>();
					}
					else
					{
						nextEntity = scene->GetEntityByUUID(rsc.GetParent());
					}
				}
			}
			break;
			case Input::KEY_RIGHT:
			{
				if (!m_NavData.IsVertical && m_NavData.IsLayout)
				{
					if (rsc.GetParent() == idc.ID) // Canvas/Root Entity
					{
						nextEntity = scene->GetEntityByUUID(rsc.GetFront());
					}
					else
					{
						if (Entity parentEntity = scene->GetEntityByUUID(rsc.GetParent()))
						{
							auto& parentRSC = scene->GetComponent<RelationshipComponent>(parentEntity);
							nextEntity = scene->GetEntityByUUID(parentRSC.GetNext(idc.ID));
						}
					}
				}
				else
				{
					if (rsc.GetParent() == idc.ID) // Canvas/Root Entity
					{
						std::vector<Entity> entities = scene->GetAllEntitiesWith<GUIComponent>();

						bool next = false;
						for (const auto& guiEntity : entities)
						{
							auto& nextIDC = scene->GetComponent<IDComponent>(guiEntity);
							if (next && scene->GetComponent<RelationshipComponent>(guiEntity).GetParent() == nextIDC.ID)
							{
								nextEntity = guiEntity;
								break;
							}

							if (nextIDC.ID == idc.ID)
								next = true;
						}
						entities.clear();
						entities = std::vector<Entity>();
					}
					else
					{
						nextEntity = scene->GetEntityByUUID(rsc.GetParent());
					}
				}
			}
			break;
			case Input::KEY_ENTER:
			{
				if (m_NavData.FocusedEntity && scene->HasComponent<GUIComponent>(m_NavData.FocusedEntity)) // Focusing on GUIComponent Entity
				{
					auto& focusedGUIC = scene->GetComponent<GUIComponent>(m_NavData.FocusedEntity);

					if (m_NavData.SelectedEntity) // Check if old selected exists
					{
						if (m_NavData.SelectedEntity == m_NavData.FocusedEntity) // Focused is already selected, Unselect focused
						{
							Select(scene, Entity());
							Focus(scene, m_NavData.FocusedEntity);
							focusedGUIC.LastState = GUIState::Enabled; // override state set in Focus()
						}
						else // Old selected exists & is different from focused
						{
							bool unselectOld = true;
							if (scene->HasComponent<GUICheckboxComponent>(m_NavData.SelectedEntity))
								unselectOld = false; // Don't unselect checkbox

							Select(scene, m_NavData.FocusedEntity, unselectOld);
						}
					}
					else // Selected doesn't exist, ready to select focused
						Select(scene, m_NavData.FocusedEntity, false);
				}
				return;
			}
			default:
				break;
			}
		}

		if (nextEntity)
		{
			// Prepare last focused
			if (m_NavData.FocusedEntity)
			{
				auto& focusedGUIC = scene->GetComponent<GUIComponent>(m_NavData.FocusedEntity);

				if (m_NavData.SelectedEntity && m_NavData.SelectedEntity == m_NavData.FocusedEntity)
				{
					if (!scene->HasComponent<GUICheckboxComponent>(m_NavData.SelectedEntity))
					{
						Unselect(scene);
						focusedGUIC.LastState = GUIState::Enabled;
					}
				}

				// Restore last state to currently focused, before unfocus
				if (scene->HasComponent<GUICheckboxComponent>(m_NavData.FocusedEntity) && focusedGUIC.CurrentState == GUIState::Selected)
					focusedGUIC.LastState = focusedGUIC.CurrentState;
				else
					focusedGUIC.CurrentState = focusedGUIC.LastState;

			}

			// Prepare next focused
			auto& guiC = scene->GetComponent<GUIComponent>(nextEntity);
			if (guiC.CurrentState == GUIState::Selected) // If next entity is selected, set as currently selected
			{
				bool unselect = true;
				if (m_NavData.SelectedEntity && scene->HasComponent<GUICheckboxComponent>(m_NavData.SelectedEntity))
					unselect = false;

				Select(scene, nextEntity, unselect);
			}

			Focus(scene, nextEntity);
		}
	}
	bool GUILayer::OnKeyPressed(KeyPressedEvent& e)
	{
		bool ret = false;
		Ref<Scene> scene = Project::GetRuntimeScene();
		if (scene && !scene->IsStopped() && Application::IsFramebufferHovered())
		{
			if (m_NavData.SelectedEntity && scene->HasComponent<GUIComponent>(m_NavData.SelectedEntity))
			{
				bool unselect = false;
				// TODO : Add cursor to input char at an index
				// Calculate input field size and cursor position within
				if (scene->HasComponent<GUIInputFieldComponent>(m_NavData.SelectedEntity))
				{
					auto& guiIFC = scene->GetComponent<GUIInputFieldComponent>(m_NavData.SelectedEntity);
					switch (e.GetKeyCode())
					{
					case Input::KEY_ESCAPE:
					{
						Entity newFocused = m_NavData.SelectedEntity;
						Unselect(scene);
						Focus(scene, newFocused);
					}
						break;
					case Input::KEY_BACKSPACE:
						Application::SubmitToMainAppThread([&guiIFC]() { guiIFC.Text.pop_back(); });
						break;
					default:
						Application::SubmitToMainAppThread([e, &guiIFC]() { guiIFC.Text.push_back(Input::GetCharFromKeyCode(e.GetKeyCode())); });
						break;
					}
				}
				else if (scene->HasComponent<GUISliderComponent>(m_NavData.SelectedEntity))
				{
					auto& guiSC = scene->GetComponent<GUISliderComponent>(m_NavData.SelectedEntity);
					bool isVertical = guiSC.Direction == SliderDirection::Top || guiSC.Direction == SliderDirection::Middle || guiSC.Direction == SliderDirection::Bottom;
					switch (e.GetKeyCode())
					{
					case Input::KeyCode::KEY_UP:
					{
						if (isVertical && guiSC.Fill <= 1.0f)
							guiSC.Fill += 0.25f;
					}
					break;
					case Input::KeyCode::KEY_DOWN:
					{
						if (isVertical && guiSC.Fill >= 0.0f)
							guiSC.Fill -= 0.25f;
					}
					break;
					case Input::KeyCode::KEY_LEFT:
					{
						if (!isVertical)
						{
							if (guiSC.Direction == SliderDirection::Left && guiSC.Fill <= 1.0f)
								guiSC.Fill += 0.25f;
							else if (guiSC.Direction == SliderDirection::Center && guiSC.Fill >= 0.0f)
								guiSC.Fill -= 0.25f;
							else if (guiSC.Direction == SliderDirection::Right && guiSC.Fill >= 0.0f)
								guiSC.Fill -= 0.25f;
						}
					}
					break;
					case Input::KeyCode::KEY_RIGHT:
					{
						if (!isVertical)
						{
							if (guiSC.Direction == SliderDirection::Left && guiSC.Fill >= 0.0f)
								guiSC.Fill -= 0.25f;
							else if (guiSC.Direction == SliderDirection::Center && guiSC.Fill >= 0.0f)
								guiSC.Fill -= 0.25f;
							else if (guiSC.Direction == SliderDirection::Right && guiSC.Fill <= 1.0f)
								guiSC.Fill += 0.25f;
						}
					}
					break;
					case Input::KEY_ENTER:
					{
						Entity newFocused = m_NavData.SelectedEntity;
						Unselect(scene);
						Focus(scene, newFocused);
						break;
					}
					default:
						break;
					}
				}

				// Sends Event to Scripting
				ret = Project::EventScene(e, m_NavData.SelectedEntity);
			}
			else if (m_NavData.FocusedEntity)
				TraverseGUIEntity(e, scene, m_NavData.FocusedEntity);
			else
			{
				if (Entity canvasEntity = scene->GetGUICanvasEntity(p_Config.ID))
				{
					auto& ac = scene->GetComponent<ActiveComponent>(canvasEntity);
					if(ac.Active)
						Focus(scene, canvasEntity);
				}
			}

		}
		return ret;
	}

	bool GUILayer::OnMousePressed(MousePressedEvent& e)
	{
		Ref<Scene> scene = Project::GetRuntimeScene();
		if (scene && !scene->IsStopped() && Application::IsFramebufferHovered())
		{
			switch (e.GetButton())
			{
			case Input::MouseCode::MOUSE_BUTTON_1:
			{
				if (m_NavData.HoveredEntity)
				{
					// Select new entity
					auto& tc = scene->GetComponent<TagComponent>(m_NavData.HoveredEntity);
					auto& rc = scene->GetComponent<RenderComponent>(m_NavData.HoveredEntity);
					if (rc.IDHandled(p_Config.ID) && tc.TagID == Project::GetTagFromStr("UI") && scene->HasComponent<GUIComponent>(m_NavData.HoveredEntity))
					{
						auto& nc = scene->GetComponent<NameComponent>(m_NavData.HoveredEntity);
						auto& guiC = scene->GetComponent<GUIComponent>(m_NavData.HoveredEntity);
						if (m_NavData.SelectedEntity)
						{
							// Select currently hovered Entity w/o unselecting Checkbox if exists
							bool unselectOld = true;
							Entity newSelected = Entity();
							if (m_NavData.SelectedEntity != m_NavData.HoveredEntity)
							{
								if (scene->HasComponent<GUICheckboxComponent>(m_NavData.SelectedEntity))
									unselectOld = false;

								newSelected = m_NavData.HoveredEntity;
							}
							else
							{
								if (scene->HasComponent<GUISliderComponent>(m_NavData.SelectedEntity))
								{
									unselectOld = false;
									newSelected = m_NavData.SelectedEntity;
								}
							}
							Select(scene, newSelected, unselectOld);
						}
						else if (guiC.CurrentState == GUIState::Selected) // Entity is Selected, but isn't currently handled selected
						{
							guiC.CurrentState = GUIState::Hovered;
						}
						else
							Select(scene, m_NavData.HoveredEntity, false);

						// Handle C# Script Event on Entity, if Script exists
						return Project::EventScene(e, m_NavData.HoveredEntity);
					}
				}
				else if (m_NavData.SelectedEntity) // No new hovered entity, but selected still exists
				{
					// Unselect old w/o unselecting Checkbox if exists
					bool unselectOld = true;
					if (scene->HasComponent<GUICheckboxComponent>(m_NavData.SelectedEntity))
						unselectOld = false;
					
					Select(scene, Entity(), unselectOld);
				}
				
				if (m_NavData.FocusedEntity && !m_NavData.HoveredEntity)
				{
					auto& guiC = scene->GetComponent<GUIComponent>(m_NavData.FocusedEntity);

					Unfocus(scene);
					guiC.LastState = GUIState::Enabled;
				}
							
			}
				break;
			case Input::MouseCode::MOUSE_BUTTON_2:
			{
			}
				break;
			default:
				break;
			}
		}
		return false;
	}

	bool GUILayer::OnMouseScrolled(MouseScrolledEvent& e)
	{
		// TODO : Set Event variable & Scroll Selected Entity if exists
		return false;
	}

	void GUILayer::Hover(Ref<Scene> scene, Entity entity, bool unhoverOld)
	{
		if (unhoverOld)
			Unhover(scene);

		if (scene && entity && scene->HasComponent<GUIComponent>(entity))
		{
			auto& guiC = scene->GetComponent<GUIComponent>(entity);
			if (guiC.CurrentState == GUIState::Enabled)
			{
				auto& nc = scene->GetComponent<NameComponent>(entity);
				//GE_CORE_INFO("Hovered Entity, \"{0}\" , on GUILayer.", nc.Name.c_str());
				guiC.LastState = guiC.CurrentState;
				guiC.CurrentState = GUIState::Hovered;
			}
		}
		m_NavData.HoveredEntity = entity;
	}
	void GUILayer::Unhover(Ref<Scene> scene)
	{
		if (!scene || !m_NavData.HoveredEntity)
			return;

		if (scene->HasComponent<GUIComponent>(m_NavData.HoveredEntity))
		{
			auto& guiC = scene->GetComponent<GUIComponent>(m_NavData.HoveredEntity);
			if (guiC.CurrentState == GUIState::Hovered)
			{
				auto& nc = scene->GetComponent<NameComponent>(m_NavData.HoveredEntity);
				//GE_CORE_INFO("Unhovered Entity, \"{0}\" , on GUILayer.", nc.Name.c_str());
				guiC.CurrentState = guiC.LastState;
			}
		}

		m_NavData.HoveredEntity = Entity();
	}

	void GUILayer::Focus(Ref<Scene> scene, Entity entity, bool unfocusOld)
	{
		if (unfocusOld)
			Unfocus(scene);

		if (scene && entity && scene->HasComponent<GUIComponent>(entity))
		{
			auto& guiC = scene->GetComponent<GUIComponent>(entity);
			if (guiC.CurrentState == GUIState::Enabled || guiC.CurrentState == GUIState::Selected)
			{
				auto& nc = scene->GetComponent<NameComponent>(entity);
				//GE_CORE_INFO("Focused Entity, \"{0}\" , on GUILayer.", nc.Name.c_str());
				guiC.LastState = guiC.CurrentState;
				guiC.CurrentState = GUIState::Focused;
			}
		}
		m_NavData.FocusedEntity = entity;
	}
	void GUILayer::Unfocus(Ref<Scene> scene)
	{
		if (!scene || !m_NavData.FocusedEntity)
			return;

		if (scene->HasComponent<GUIComponent>(m_NavData.FocusedEntity))
		{
			auto& guiC = scene->GetComponent<GUIComponent>(m_NavData.FocusedEntity);
			if (guiC.CurrentState == GUIState::Focused)
			{
				auto& nc = scene->GetComponent<NameComponent>(m_NavData.FocusedEntity);
				//GE_CORE_INFO("Unfocused Entity, \"{0}\" , on GUILayer.", nc.Name.c_str());
				guiC.CurrentState = guiC.LastState;
			}
		}
		m_NavData.FocusedEntity = Entity();
	}
		
	void GUILayer::Select(Ref<Scene> scene, Entity entity, bool unselectOld)
	{
		if (unselectOld)
			Unselect(scene);

		if (scene && entity && scene->HasComponent<GUIComponent>(entity))
		{
			auto& guiC = scene->GetComponent<GUIComponent>(entity);
			if (guiC.CurrentState == GUIState::Hovered || guiC.CurrentState == GUIState::Focused)
			{
				auto& nc = scene->GetComponent<NameComponent>(entity);
 				//GE_CORE_INFO("Selected Entity, \"{0}\" , on GUILayer.", nc.Name.c_str());
				guiC.CurrentState = GUIState::Selected;
			}
		}
		m_Steps = 0;
		m_NavData.SelectedEntity = entity;
	}
	void GUILayer::Unselect(Ref<Scene> scene)
	{
		if (!scene || !m_NavData.SelectedEntity)
			return;

		if (scene->HasComponent<GUIComponent>(m_NavData.SelectedEntity))
		{
			auto& guiC = scene->GetComponent<GUIComponent>(m_NavData.SelectedEntity);
			if (guiC.CurrentState == GUIState::Selected)
			{
				auto& nc = scene->GetComponent<NameComponent>(m_NavData.SelectedEntity);
				//GE_CORE_INFO("Unselected Entity, \"{0}\" , on GUILayer.", nc.Name.c_str());
				guiC.CurrentState = GUIState::Enabled;
			}
		}
		m_Steps = 0;
		m_NavData.SelectedEntity = Entity();
	}
}