#include "GE/GEpch.h"

#include "Scripting.h"

#include "GE/Asset/Assets/Scene/Scene.h"
#include "GE/Audio/AudioManager.h"

#include "GE/Core/Application/Application.h"
#include "GE/Core/FileSystem/FileSystem.h"
#include "GE/Core/Input/Input.h"

#include "GE/Physics/Physics.h"
#include "GE/Project/Project.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>
#pragma warning(pop)

#include <glm/glm.hpp>

namespace GE
{
#define GE_ADD_INTERNAL_CALL(Name) mono_add_internal_call("GE.InternalCalls::" #Name, Name)
	
#pragma region Internal Call Declarations

#pragma region Log Internal Calls

	static void Log_Core_Info(MonoString* debugMessage)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			std::string debug = Scripting::MonoStringToString(debugMessage);
			GE_CORE_INFO(debug);
		}
	}

	static void Log_Core_Trace(MonoString* debugMessage)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			std::string debug = Scripting::MonoStringToString(debugMessage);
			GE_CORE_TRACE(debug);
		}
	}

	static void Log_Core_Warn(MonoString* debugMessage)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			std::string debug = Scripting::MonoStringToString(debugMessage);
			GE_CORE_WARN(debug);
		}

	}

	static void Log_Core_Error(MonoString* debugMessage)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			std::string debug = Scripting::MonoStringToString(debugMessage);
			GE_CORE_ERROR(debug);
		}

	}

	static void Log_Core_Assert(MonoObject* object, MonoString* debugMessage)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			std::string debug = Scripting::MonoStringToString(debugMessage);
			GE_CORE_ASSERT(!object, debug);
		}
	}

#pragma endregion

#pragma region Input Internal Calls
	static bool Input_IsKeyDown(Input::KeyCode keyCode)
	{
		return Application::IsKeyPressed(keyCode);
	}

	static bool Input_IsMouseDown(Input::MouseCode mouseCode)
	{
		return Application::IsMousePressed(mouseCode);
	}
#pragma endregion

#pragma region Entity & Component Internal Calls

	static bool Entity_IsHovered(UUID uuid)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			if (Entity hoveredEntity = Application::GetHoveredEntity())
				return scene->GetComponent<IDComponent>(hoveredEntity).ID == uuid;
		}
		return false;
	}

	static bool Entity_HasComponent(UUID uuid, MonoReflectionType* componentType)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			if (Entity entity = scene->GetEntityByUUID(uuid))
			{
				MonoType* type = mono_reflection_type_get_type(componentType);
				GE_CORE_ASSERT(Scripting::HasComponentFunc(type), "Unable to find Component Type.");
				return Scripting::GetHasComponentFunc(type, scene, entity);
			}
		}
		GE_CORE_ERROR("Scripting: Could not determine if Entity has Component.");
		return false;
	}

	static uint64_t Entity_FindEntityByName(MonoString* name)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			std::string nameStr = Scripting::MonoStringToString(name);
			if (Entity entity = scene->GetEntityByName(nameStr))
				return scene->GetComponent<IDComponent>(entity).ID;
		}
		return 0;
	}

	static MonoObject* Entity_GetScriptInstance(uint64_t uuid)
	{
		if(Ref<ScriptInstance> instance = Scripting::GetEntityInstance(uuid))
			return instance->GetMonoObject();
		return nullptr;
	}
#pragma region TransformComponent

	static void TransformComponent_GetTranslation(UUID uuid, glm::vec3* translation)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<TransformComponent>(entity))
			{
				auto& trsc = scene->GetComponent<TransformComponent>(entity);
				*translation = trsc.Translation;
			}
		}
	}

	static void TransformComponent_SetTranslation(UUID uuid, glm::vec3* translation)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<TransformComponent>(entity))
			{
				auto& trsc = scene->GetComponent<TransformComponent>(entity);
				trsc.Translation = *translation;

				// Special case if Entity has Camera
				// Ensures SceneCamera moves with parent
				scene->SyncCamera(entity, trsc.GetOffsetTranslation(), trsc.Rotation);
			}
		}
	}

	static void TransformComponent_GetPivot(UUID uuid, glm::vec3* pivot)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<TransformComponent>(entity))
			{
				auto& tc = scene->GetComponent<TransformComponent>(entity);
				*pivot = tc.GetPivotOffset();
			}
		}
	}
#pragma endregion
#pragma region ActiveComponent
	static void ActiveComponent_GetActive(UUID uuid, bool* active)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<ActiveComponent>(entity))
			{
				auto& ac = scene->GetComponent<ActiveComponent>(entity);
				*active = ac.Active;
			}
		}
	}

	static void ActiveComponent_SetActive(UUID uuid, bool* active)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<ActiveComponent>(entity))
			{
				auto& ac = scene->GetComponent<ActiveComponent>(entity);
				ac.Active = *active;

			}
		}
	}

	static void ActiveComponent_GetHidden(UUID uuid, bool* hidden)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<ActiveComponent>(entity))
			{
				auto& ac = scene->GetComponent<ActiveComponent>(entity);
				*hidden = ac.Hidden;
			}
		}
	}

	static void ActiveComponent_SetHidden(UUID uuid, bool* hidden)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<ActiveComponent>(entity))
			{
				auto& ac = scene->GetComponent<ActiveComponent>(entity);
				ac.Hidden = *hidden;

			}
		}
	}
#pragma endregion

#pragma region Audio Component

	static void AudioSourceComponent_GetLoop(UUID uuid, bool* loop)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<AudioSourceComponent>(entity))
			{
				auto& asc = scene->GetComponent<AudioSourceComponent>(entity);
				*loop = asc.Loop;
			}
		}
	}

	static void AudioSourceComponent_SetLoop(UUID uuid, bool* loop)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<AudioSourceComponent>(entity))
			{
				auto& asc = scene->GetComponent<AudioSourceComponent>(entity);
				asc.Loop = *loop;
			}
		}
	}

	static void AudioSourceComponent_GetPitch(UUID uuid, float* pitch)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<AudioSourceComponent>(entity))
			{
				auto& asc = scene->GetComponent<AudioSourceComponent>(entity);
				*pitch = asc.Pitch;
			}
		}
	}

	static void AudioSourceComponent_SetPitch(UUID uuid, float* pitch)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<AudioSourceComponent>(entity))
			{
				auto& asc = scene->GetComponent<AudioSourceComponent>(entity);
				asc.Pitch = *pitch;
			}
		}
	}

	static void AudioSourceComponent_GetGain(UUID uuid, float* gain)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<AudioSourceComponent>(entity))
			{
				auto& asc = scene->GetComponent<AudioSourceComponent>(entity);
				*gain = asc.Gain;
			}
		}
	}

	static void AudioSourceComponent_SetGain(UUID uuid, float* gain)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<AudioSourceComponent>(entity))
			{
				auto& asc = scene->GetComponent<AudioSourceComponent>(entity);
				asc.Gain = *gain;
			}
		}
	}

	static void AudioSourceComponent_Play(UUID uuid, glm::vec3* translation = nullptr, glm::vec3* velocity = nullptr)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<AudioSourceComponent>(entity))
			{
				glm::vec3 position = glm::vec3(0.0);
				if (translation)
					position = *translation;
				else if (scene->HasComponent<TransformComponent>(entity))
				{
					auto& trsc = scene->GetComponent<TransformComponent>(entity);
					position = trsc.Translation;
				}

				glm::vec3 v = glm::vec3(0.0);
				if (velocity)
					v = *velocity;
				else if (scene->HasComponent<Rigidbody2DComponent>(entity))
				{
					auto& rb2D = scene->GetComponent<Rigidbody2DComponent>(entity);
					if (rb2D.RuntimeBody)
					{
						b2Body* body = (b2Body*)rb2D.RuntimeBody;
						const auto& lv = body->GetLinearVelocity();
						v = glm::vec3(lv.x, lv.y, 0.0);
					}
				}

				auto& asc = scene->GetComponent<AudioSourceComponent>(entity);
				asc.Play(position, v);
			}
		}
	}
#pragma endregion

#pragma region Rigidbody2D Component

	// Point is World position
	static void Rigidbody2DComponent_ApplyLinearImpulse(UUID uuid, glm::vec2* impluse, glm::vec2* point, bool wake)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<Rigidbody2DComponent>(entity))
			{
				auto& rbc = scene->GetComponent<Rigidbody2DComponent>(entity);
				b2Body* body = (b2Body*)rbc.RuntimeBody;
				body->ApplyLinearImpulse(b2Vec2(impluse->x, impluse->y), b2Vec2(point->x, point->y), wake);

			}
		}
	}

	static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID uuid, glm::vec2* impluse, bool wake)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<Rigidbody2DComponent>(entity))
			{
				auto& rbc = scene->GetComponent<Rigidbody2DComponent>(entity);
				b2Body* body = (b2Body*)rbc.RuntimeBody;
				body->ApplyLinearImpulseToCenter(b2Vec2(impluse->x, impluse->y), wake);

			}
		}
	}

	static void Rigidbody2DComponent_GetLinearVelocity(UUID uuid, glm::vec2* velocity)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<Rigidbody2DComponent>(entity))
			{
				auto& rbc = scene->GetComponent<Rigidbody2DComponent>(entity);
				b2Body* body = (b2Body*)rbc.RuntimeBody;
				const b2Vec2& linearVelocity = body->GetLinearVelocity();
				*velocity = glm::vec2(linearVelocity.x, linearVelocity.y);
			}
		}
	}

	static Rigidbody2DComponent::BodyType Rigidbody2DComponent_GetType(UUID entityID)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(entityID);
			if (entity && scene->HasComponent<Rigidbody2DComponent>(entity))
			{
				auto& rb2d = scene->GetComponent<Rigidbody2DComponent>(entity);
				b2Body* body = (b2Body*)rb2d.RuntimeBody;
				return Physics::Rigidbody2DTypeFromBox2DBody(body->GetType());
			}
		}
		return Rigidbody2DComponent::BodyType::Static;
	}

	static void Rigidbody2DComponent_SetType(UUID entityID, Rigidbody2DComponent::BodyType bodyType)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(entityID);
			if (entity && scene->HasComponent<Rigidbody2DComponent>(entity))
			{
				auto& rb2d = scene->GetComponent<Rigidbody2DComponent>(entity);
				b2Body* body = (b2Body*)rb2d.RuntimeBody;
				body->SetType(Physics::Rigidbody2DTypeToBox2DBody(bodyType));
			}
		}
	}
#pragma endregion

#pragma region Rendering

	static void SpriteRendererComponent_GetColor(UUID uuid, glm::vec4* color)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<SpriteRendererComponent>(entity))
			{
				auto& src = scene->GetComponent<SpriteRendererComponent>(entity);
				*color = src.Color;
			}
		}
	}

	static void SpriteRendererComponent_SetColor(UUID uuid, glm::vec4* color)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<SpriteRendererComponent>(entity))
			{
				auto& src = scene->GetComponent<SpriteRendererComponent>(entity);
				src.Color = *color;
			}
		}
	}


#pragma region Text Component
	static MonoString* TextRendererComponent_GetText(UUID uuid)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<TextRendererComponent>(entity))
			{
				auto& trc = scene->GetComponent<TextRendererComponent>(entity);
				return Scripting::StringToMonoString(trc.Text.c_str());
			}
		}
		return nullptr;
	}

	static void TextRendererComponent_SetText(UUID uuid, MonoString* textString)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<TextRendererComponent>(entity))
			{
				auto& trc = scene->GetComponent<TextRendererComponent>(entity);
				trc.Text = Scripting::MonoStringToString(textString);
			}
		}
	}

	static void TextRendererComponent_GetTextColor(UUID uuid, glm::vec4* textColor)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<TextRendererComponent>(entity))
			{
				auto& trc = scene->GetComponent<TextRendererComponent>(entity);
				*textColor = trc.TextColor;
			}
		}
	}

	static void TextRendererComponent_SetTextColor(UUID uuid, glm::vec4* textColor)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<TextRendererComponent>(entity))
			{
				auto& trc = scene->GetComponent<TextRendererComponent>(entity);
				trc.TextColor = *textColor;
			}
		}
	}

	static void TextRendererComponent_GetBGColor(UUID uuid, glm::vec4* bgColor)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<TextRendererComponent>(entity))
			{
				auto& trc = scene->GetComponent<TextRendererComponent>(entity);
				*bgColor = trc.BGColor;
			}
		}
	}

	static void TextRendererComponent_SetBGColor(UUID uuid, glm::vec4* bgColor)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<TextRendererComponent>(entity))
			{
				auto& trc = scene->GetComponent<TextRendererComponent>(entity);
				trc.BGColor = *bgColor;
			}
		}
	}

	static float TextRendererComponent_GetLineHeight(UUID uuid)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<TextRendererComponent>(entity))
			{
				auto& trc = scene->GetComponent<TextRendererComponent>(entity);
				return trc.LineHeightOffset;
			}
		}
		return 0.0f;
	}

	static void TextRendererComponent_SetLineHeight(UUID uuid, float lineHeight)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<TextRendererComponent>(entity))
			{
				auto& trc = scene->GetComponent<TextRendererComponent>(entity);
				trc.LineHeightOffset = lineHeight;
			}
		}
	}

	static float TextRendererComponent_GetLineSpacing(UUID uuid)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<TextRendererComponent>(entity))
			{
				auto& trc = scene->GetComponent<TextRendererComponent>(entity);
				return trc.KerningOffset;
			}
		}
		return 0.0f;
	}

	static void TextRendererComponent_SetLineSpacing(UUID uuid, float lineSpacing)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<TextRendererComponent>(entity))
			{
				auto& trc = scene->GetComponent<TextRendererComponent>(entity);
				trc.KerningOffset = lineSpacing;
			}
		}
	}

#pragma endregion

#pragma region GUI

#pragma region Canvas
	static void GUICanvasComponent_GetControlMouse(UUID uuid, bool* control)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUICanvasComponent>(entity))
			{
				auto& guiCC = scene->GetComponent<GUICanvasComponent>(entity);
				*control = guiCC.ControlMouse;
			}
		}
	}

	static void GUICanvasComponent_SetControlMouse(UUID uuid, bool* control)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUICanvasComponent>(entity))
			{
				auto& guiCC = scene->GetComponent<GUICanvasComponent>(entity);
				guiCC.ControlMouse = *control;
			}
		}
	}

	static void GUICanvasComponent_GetShowMouse(UUID uuid, bool* show)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUICanvasComponent>(entity))
			{
				auto& guiCC = scene->GetComponent<GUICanvasComponent>(entity);
				*show = guiCC.ShowMouse;
			}
		}
	}

	static void GUICanvasComponent_SetShowMouse(UUID uuid, bool* show)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUICanvasComponent>(entity))
			{
				auto& guiCC = scene->GetComponent<GUICanvasComponent>(entity);
				guiCC.ShowMouse = *show;
			}
		}
	}

	static void GUICanvasComponent_GetMode(UUID uuid, int* mode)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUICanvasComponent>(entity))
			{
				auto& guiCC = scene->GetComponent<GUICanvasComponent>(entity);
				*mode = (int)guiCC.Mode;
			}
		}
	}

	static void GUICanvasComponent_SetMode(UUID uuid, int* mode)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUICanvasComponent>(entity))
			{
				auto& guiCC = scene->GetComponent<GUICanvasComponent>(entity);
				guiCC.Mode = (CanvasMode)*mode;
			}
		}
	}
#pragma endregion

#pragma region Button

	static void GUIButtonComponent_GetEnabledColor(UUID uuid, glm::vec4* color)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUIButtonComponent>(entity))
			{
				auto& guiBC = scene->GetComponent<GUIButtonComponent>(entity);
				*color = guiBC.EnabledColor;
			}
		}
	}

	static void GUIButtonComponent_SetEnabledColor(UUID uuid, glm::vec4* color)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUIButtonComponent>(entity))
			{
				auto& guiBC = scene->GetComponent<GUIButtonComponent>(entity);
				guiBC.EnabledColor = *color;
			}
		}
	}

	static void GUIButtonComponent_GetHoveredColor(UUID uuid, glm::vec4* hoveredColor)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUIButtonComponent>(entity))
			{
				auto& guiBC = scene->GetComponent<GUIButtonComponent>(entity);
				*hoveredColor = guiBC.HoveredColor;
			}
		}
	}

	static void GUIButtonComponent_SetHoveredColor(UUID uuid, glm::vec4* hoveredColor)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUIButtonComponent>(entity))
			{
				auto& guiBC = scene->GetComponent<GUIButtonComponent>(entity);
				guiBC.HoveredColor = *hoveredColor;
			}
		}
	}
#pragma endregion

#pragma region InputField

	static MonoString* GUIInputFieldComponent_GetText(UUID uuid)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUIInputFieldComponent>(entity))
			{
				auto& guiIFC = scene->GetComponent<GUIInputFieldComponent>(entity);
				return Scripting::StringToMonoString(guiIFC.Text.c_str());
			}
		}
		return nullptr;
	}

	static void GUIInputFieldComponent_SetText(UUID uuid, MonoString* textString)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUIInputFieldComponent>(entity))
			{
				auto& guiIFC = scene->GetComponent<GUIInputFieldComponent>(entity);
				guiIFC.Text = Scripting::MonoStringToString(textString);
			}
		}
	}

	static void GUIInputFieldComponent_GetTextColor(UUID uuid, glm::vec4* textColor)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUIInputFieldComponent>(entity))
			{
				auto& guiIFC = scene->GetComponent<GUIInputFieldComponent>(entity);
				*textColor = guiIFC.TextColor;
			}
		}
	}

	static void GUIInputFieldComponent_SetTextColor(UUID uuid, glm::vec4* textColor)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUIInputFieldComponent>(entity))
			{
				auto& guiIFC = scene->GetComponent<GUIInputFieldComponent>(entity);
				guiIFC.TextColor = *textColor;
			}
		}
	}

	static void GUIInputFieldComponent_GetBGColor(UUID uuid, glm::vec4* bgColor)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUIInputFieldComponent>(entity))
			{
				auto& guiIFC = scene->GetComponent<GUIInputFieldComponent>(entity);
				*bgColor = guiIFC.BGColor;
			}
		}
	}

	static void GUIInputFieldComponent_SetBGColor(UUID uuid, glm::vec4* bgColor)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUIInputFieldComponent>(entity))
			{
				auto& guiIFC = scene->GetComponent<GUIInputFieldComponent>(entity);
				guiIFC.BGColor = *bgColor;
			}
		}
	}

	static float GUIInputFieldComponent_GetLineHeight(UUID uuid)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUIInputFieldComponent>(entity))
			{
				auto& guiIFC = scene->GetComponent<GUIInputFieldComponent>(entity);
				return guiIFC.LineHeightOffset;
			}
		}
		return 0.0f;
	}

	static void GUIInputFieldComponent_SetLineHeight(UUID uuid, float lineHeight)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUIInputFieldComponent>(entity))
			{
				auto& guiIFC = scene->GetComponent<GUIInputFieldComponent>(entity);
				guiIFC.LineHeightOffset = lineHeight;
			}
		}
	}

	static float GUIInputFieldComponent_GetLineSpacing(UUID uuid)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUIInputFieldComponent>(entity))
			{
				auto& guiIFC = scene->GetComponent<GUIInputFieldComponent>(entity);
				return guiIFC.KerningOffset;
			}
		}
		return 0.0f;
	}

	static void GUIInputFieldComponent_SetLineSpacing(UUID uuid, float lineSpacing)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUIInputFieldComponent>(entity))
			{
				auto& guiIFC = scene->GetComponent<GUIInputFieldComponent>(entity);
				guiIFC.KerningOffset = lineSpacing;
			}
		}
	}

#pragma endregion

#pragma region Slider

	static void GUISliderComponent_GetFill(UUID uuid, float* fill)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUISliderComponent>(entity))
			{
				auto& guiSC = scene->GetComponent<GUISliderComponent>(entity);
				*fill = guiSC.Fill;
			}
		}
	}

	static void GUISliderComponent_SetFill(UUID uuid, float* fill)
	{
		if (Ref<Scene> scene = Project::GetRuntimeScene())
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			if (entity && scene->HasComponent<GUISliderComponent>(entity))
			{
				auto& guiSC = scene->GetComponent<GUISliderComponent>(entity);
				guiSC.Fill = *fill;

			}
		}
	}
#pragma endregion

#pragma endregion

#pragma endregion

#pragma endregion

#pragma endregion

#pragma region Scripting

	Scripting::Data Scripting::s_Data = Scripting::Data();

	std::unordered_map<MonoType*, std::function<bool(Ref<Scene>, Entity)>> Scripting::s_HasComponentsFuncs = std::unordered_map<MonoType*, std::function<bool(Ref<Scene>, Entity)>>();

	std::unordered_map<std::string, ScriptField::Type> Scripting::s_ScriptFieldTypeNames =
	{
		{ "System.Char",	ScriptField::Type::Char		},
		{ "System.Int",		ScriptField::Type::Int		},
		{ "System.UInt",	ScriptField::Type::UInt		},
		{ "System.Single",	ScriptField::Type::Float	},
		{ "System.Byte",	ScriptField::Type::Byte		},
		{ "System.Boolean",	ScriptField::Type::Bool		},
		{ "GE.Vector2",		ScriptField::Type::Vector2	},
		{ "GE.Vector3",		ScriptField::Type::Vector3	},
		{ "GE.Vector4",		ScriptField::Type::Vector4	},
		{ "GE.Entity",		ScriptField::Type::Entity	}
	};

#pragma region Helpers

	void Scripting::PrintMonoAssemblyTypes(MonoAssembly* assembly)
	{
		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionTable);

		GE_CORE_TRACE("Assembly Types");
		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			GE_CORE_TRACE("\n\tNamespace : {0}\n\tClassName: {1}", nameSpace, name);
		}
	}

	ScriptField::Type Scripting::MonoTypeToScriptFieldType(MonoType* monoType)
	{
		const char* typeName = mono_type_get_name(monoType);
		if (s_ScriptFieldTypeNames.find(typeName) == s_ScriptFieldTypeNames.end())
			return ScriptField::Type::None;

		return s_ScriptFieldTypeNames.at(typeName);
	}

	MonoString* Scripting::StringToMonoString(const char* string)
	{
		return mono_string_new(s_Data.AppDomain, string);
	}

	std::string Scripting::MonoStringToString(MonoString* string)
	{
		char* cStr = mono_string_to_utf8(string);
		std::string str(cStr);
		mono_free(cStr);
		return str;
	}

	ScriptField::Type Scripting::StringToScriptFieldType(const std::string& typeStr)
	{
		if (typeStr == "None")		return ScriptField::Type::None;
		if (typeStr == "Char")		return ScriptField::Type::Char;
		if (typeStr == "Int")		return ScriptField::Type::Int;
		if (typeStr == "UInt")		return ScriptField::Type::UInt;
		if (typeStr == "Float")		return ScriptField::Type::Float;
		if (typeStr == "Byte")		return ScriptField::Type::Byte;
		if (typeStr == "Boolean")	return ScriptField::Type::Bool;
		if (typeStr == "Vector2")	return ScriptField::Type::Vector3;
		if (typeStr == "Vector3")	return ScriptField::Type::Vector3;
		if (typeStr == "Vector4")	return ScriptField::Type::Vector4;
		if (typeStr == "Entity")	return ScriptField::Type::Entity;

		GE_CORE_ASSERT(false, "Unknown ScriptField Type.")
			return ScriptField::Type::None;
	}

	const char* Scripting::ScriptFieldTypeToString(ScriptField::Type fieldType)
	{
		switch (fieldType)
		{
		case GE::ScriptField::Type::None:
			return "None";
			break;
		case GE::ScriptField::Type::Char:
			return "Char";
			break;
		case GE::ScriptField::Type::Int:
			return "Int";
			break;
		case GE::ScriptField::Type::UInt:
			return "UInt";
			break;
		case GE::ScriptField::Type::Float:
			return "Float";
			break;
		case GE::ScriptField::Type::Byte:
			return "Byte";
			break;
		case GE::ScriptField::Type::Bool:
			return "Boolean";
			break;
		case GE::ScriptField::Type::Vector2:
			return "Vector2";
			break;
		case GE::ScriptField::Type::Vector3:
			return "Vector3";
			break;
		case GE::ScriptField::Type::Vector4:
			return "Vector4";
			break;
		case GE::ScriptField::Type::Entity:
			return "Entity";
			break;
		}
		return "<Invalid>";
	}
#pragma endregion

	Ref<Script> Scripting::CreateOrReloadScript(const std::string& nameSpace, const std::string& className)
	{
		Ref<Script> script = nullptr;
		if (ScriptExists(className))
		{
			script = Scripting::GetScript(className);
		}
		else
		{
			script = CreateRef<Script>(nameSpace, className);
			s_Data.AddScript(className, script);
		}
		InstantiateScriptClass(script);
		return script;
	}

	void Scripting::InstantiateScriptClass(Ref<Script> script)
	{
		if (!script)
			return;

		script->Invalidate();
		MonoImage* monoImage = script->IsCore() ? s_Data.CoreAssemblyImage : s_Data.AppAssemblyImage;
		script->m_Class = mono_class_from_name(monoImage, script->m_ClassNamespace.c_str(), script->m_ClassName.c_str());

		void* iterator = nullptr;
		while (MonoClassField* monoField = mono_class_get_fields(script->m_Class, &iterator))
		{
			const char* fieldName = mono_field_get_name(monoField);

			uint32_t flags = mono_field_get_flags(monoField);
			if (flags & MONO_FIELD_ATTR_PUBLIC)
			{
				MonoType* monoType = mono_field_get_type(monoField);

				ScriptField::Type fieldType = MonoTypeToScriptFieldType(monoType);
				const char* fieldTypeName = ScriptFieldTypeToString(fieldType);

				script->m_Fields.emplace(fieldName, ScriptField(fieldTypeName, fieldType, monoField));
			}
		}
	}

	MonoMethod* Scripting::GetMethod(UUID scriptHandle, const Method& method)
	{
		if (Ref<Script> script = GetScript(scriptHandle))
			return GetMethod(script, method);
		return nullptr;
	}

	MonoMethod* Scripting::GetMethod(Ref<Script> script, const Method& method)
	{
		if (!script)
			return nullptr;
		return mono_class_get_method_from_name(script->m_Class, method.first.c_str(), method.second);
	}

	MonoObject* Scripting::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		MonoObject* exception = nullptr;
		mono_thread_attach(s_Data.AppDomain);
		return mono_runtime_invoke(method, instance, params, &exception);
	}

	MonoObject* Scripting::InstantiateScriptObject(UUID scriptHandle, bool isCore)
	{
		MonoObject* instance = nullptr;
		if (Ref<Script> script = GetScript(scriptHandle))
		{
			MonoDomain* domain = (isCore ? s_Data.CoreDomain : s_Data.AppDomain);
			if (!script->m_Class)
				InstantiateScriptClass(script);
			instance = mono_object_new(domain, script->m_Class);
			mono_runtime_object_init(instance);
		}
		return instance;
	}

	void Scripting::Init()
	{
		GE_CORE_INFO("Scripting Init Start");

		InitMono();
		RegisterFunctions();

		s_Data.InitializeCore(Project::GetPathToAsset(Project::GetScriptCorePath()));
		s_Data.InitializeApp(Project::GetPathToAsset(Project::GetScriptAppPath()));

		LoadAssemblyClasses();
		RegisterComponents();

		s_Data.InstantiateBaseEntity();
		GE_CORE_INFO("Scripting Init Complete");
	}

	void Scripting::Shutdown()
	{
		GE_CORE_INFO("Scripting Shutdown Start");
		OnStop();

		ShutdownMono();

		GE_CORE_INFO("Scripting Shutdown Complete");
	}

	void Scripting::OnStop()
	{
		s_Data.ClearInstances();
	}

	bool Scripting::OnEvent(Event& e, Scene* scene, Entity entity)
	{
		if (!scene || !entity)
		{ 
			GE_CORE_ERROR("Scripting::OnEvent(Event&, Scene*, Entity) - \n\t Scene or Entity don't exist.");
			return false;
		}

		UUID uuid = scene->GetComponent<IDComponent>(entity).ID;
		if (!uuid || !s_Data.EntityInstanceExists(uuid))
		{
			GE_CORE_WARN("Scripting::OnEvent(Event&, Scene*, Entity) - \n\tEntity Script Instance that does not exist.");
			return false;
		}
		return s_Data.GetEntityInstance(uuid)->InvokeOnEvent(e);
	}

	void Scripting::OnCreateScript(Scene* scene, Entity entity)
	{
		const auto& sc = scene->GetComponent<ScriptComponent>(entity);
		if (ScriptExists(sc.AssetHandle))
		{
			UUID uuid = scene->GetComponent<IDComponent>(entity).ID;
			Ref<Script> script = s_Data.GetScript(sc.AssetHandle);
			if(Ref<ScriptInstance> instance = s_Data.CreateEntityInstance(script, uuid))
				instance->InvokeOnCreate();
		}
	}

	void Scripting::OnUpdateScript(Scene* scene, Entity entity, float timestep)
	{
		if (!scene || !entity)
		{
			GE_CORE_ERROR("Scripting::OnUpdateScript(Scene*, Entity, float) - \n\t Scene or Entity don't exist.");
			return;
		}

		UUID uuid = scene->GetComponent<IDComponent>(entity).ID;
		if (!uuid || !s_Data.EntityInstanceExists(uuid))
		{
			GE_CORE_WARN("Scripting::OnUpdateScript(Scene*, Entity, Timestep) - \n\tTried to Update Entity Instance that does not exist.");
			return;
		}
		s_Data.GetEntityInstance(uuid)->InvokeOnUpdate(timestep);
	}

	void Scripting::InitMono()
	{
		if (s_Data.CoreDomain)
			return;

		GE_CORE_INFO("Mono Init Start");
		mono_set_assemblies_path("mono/lib");

		s_Data.InitializeCoreDomain();

		mono_thread_set_main(mono_thread_current());

		GE_CORE_INFO("Mono Init Complete");
	}

	void Scripting::ShutdownMono()
	{
		GE_CORE_INFO("Mono Shutdown Start");
		s_Data.ClearDomains();

		GE_CORE_INFO("Mono Shutdown Complete");

	}

	void Scripting::ReloadAssembly()
	{
		GE_CORE_TRACE("Assembly Reload Started.");
		s_Data.ResetFileWatcher();

		mono_domain_set(mono_get_root_domain(), false); // Set domain to root domain
		mono_domain_unload(s_Data.AppDomain); // Unload old app domain

		// Reload assemblies
		s_Data.InitializeCore(Project::GetPathToAsset(Project::GetScriptCorePath()));
		s_Data.InitializeApp(Project::GetPathToAsset(Project::GetScriptAppPath()));

		LoadAssemblyClasses();

		RegisterComponents();

		s_Data.InstantiateBaseEntity();
		GE_CORE_TRACE("Assembly Reload Complete");
	}

	MonoDomain* Scripting::LoadCoreDomain(const std::string& file)
	{
		return mono_jit_init(file.c_str());
	}

	MonoDomain* Scripting::LoadAppDomain(const std::string& file)
	{
		MonoDomain* appDomain = mono_domain_create_appdomain((char*)file.c_str(), nullptr);
		mono_domain_set(appDomain, true);
		return appDomain;
	}

	void Scripting::ClearCoreDomain(MonoDomain* coreDomain)
	{
		mono_jit_cleanup(coreDomain);
	}
	
	void Scripting::ClearAppDomain(MonoDomain* appDomain)
	{
		mono_domain_set(mono_get_root_domain(), false);
		mono_domain_unload(appDomain);
	}
	
	MonoAssembly* Scripting::LoadMonoAssembly(const std::filesystem::path& assemblyPath)
	{
		std::ifstream stream(assemblyPath, std::ios::binary | std::ios::ate);
		if (!stream)
		{
			GE_CORE_ASSERT(false, "Failed to open scripting file.");
			return nullptr;
		}

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint64_t size = end - stream.tellg();
		if (size == 0)
		{
			GE_CORE_ASSERT(false, "File is empty.");
			return nullptr;
		}

		char* data = new char[size];
		stream.read(data, size);
		stream.close();

		MonoImageOpenStatus monoStatus;
		MonoImage* image = mono_image_open_from_data_full(data,
			(uint32_t)size, 1, &monoStatus, 0);
		delete[] data;

		if (monoStatus != MONO_IMAGE_OK)
		{
			const char* errorMessage = mono_image_strerror(monoStatus);
			GE_CORE_ASSERT(false, errorMessage);
			return nullptr;
		}

		std::string pathString = assemblyPath.string();
		MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &monoStatus, 0);

		mono_image_close(image);
		return assembly;
	}
	
	MonoImage* Scripting::LoadMonoImage(MonoAssembly* assembly)
	{
		if (!assembly)
		{
			GE_CORE_ERROR("Scripting::LoadMonoImage(MonoAssembly*) - \n\t Could not load Image from Assembly. Assembly does not exist.");
			return nullptr;
		}
		return mono_assembly_get_image(assembly);
	}

	void Scripting::LoadAssemblyClasses()
	{
		MonoImage*& coreImage = s_Data.CoreAssemblyImage;
		MonoImage*& appImage = s_Data.AppAssemblyImage;

		const MonoTableInfo* typeDefinitionTable = mono_image_get_table_info(appImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionTable);
		MonoClass* entityClass = mono_class_from_name(coreImage, "GE", "Entity");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(appImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(appImage, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, name);
			else
				fullName = name;

			MonoClass* monoClass = mono_class_from_name(appImage, nameSpace, name);
			bool isSubClass = mono_class_is_subclass_of(monoClass, entityClass, false);
			if (!isSubClass)
				continue;

			if (Ref<Script> script = CreateOrReloadScript(nameSpace, name))
				GE_CORE_TRACE("Loaded {0}::{1} from C#", script->GetNamespace().c_str(), script->GetName().c_str());
		}
		
		GE_CORE_INFO("Scripting::LoadAssemblyClasses() - Successful.");
	}

	void Scripting::RegisterFunctions()
	{
		GE_ADD_INTERNAL_CALL(Log_Core_Info);
		GE_ADD_INTERNAL_CALL(Log_Core_Trace);
		GE_ADD_INTERNAL_CALL(Log_Core_Warn);
		GE_ADD_INTERNAL_CALL(Log_Core_Error);
		GE_ADD_INTERNAL_CALL(Log_Core_Assert);

		GE_ADD_INTERNAL_CALL(Input_IsKeyDown);
		GE_ADD_INTERNAL_CALL(Input_IsMouseDown);

#pragma region Entity

		GE_ADD_INTERNAL_CALL(Entity_IsHovered);

		GE_ADD_INTERNAL_CALL(Entity_HasComponent);
		GE_ADD_INTERNAL_CALL(Entity_FindEntityByName);
		GE_ADD_INTERNAL_CALL(Entity_GetScriptInstance);

		GE_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		GE_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		GE_ADD_INTERNAL_CALL(TransformComponent_GetPivot);

		// TODO : Set ActiveComponent
		GE_ADD_INTERNAL_CALL(ActiveComponent_GetActive);
		GE_ADD_INTERNAL_CALL(ActiveComponent_SetActive);

		GE_ADD_INTERNAL_CALL(ActiveComponent_GetHidden);
		GE_ADD_INTERNAL_CALL(ActiveComponent_SetHidden);

#pragma region Audio
		GE_ADD_INTERNAL_CALL(AudioSourceComponent_GetLoop);
		GE_ADD_INTERNAL_CALL(AudioSourceComponent_SetLoop);
		GE_ADD_INTERNAL_CALL(AudioSourceComponent_GetPitch);
		GE_ADD_INTERNAL_CALL(AudioSourceComponent_SetPitch);
		GE_ADD_INTERNAL_CALL(AudioSourceComponent_GetGain);
		GE_ADD_INTERNAL_CALL(AudioSourceComponent_SetGain);
		GE_ADD_INTERNAL_CALL(AudioSourceComponent_Play);
#pragma endregion

#pragma region Physics
		GE_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		GE_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
		GE_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetLinearVelocity);
		GE_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetType);
		GE_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetType);

#pragma endregion

#pragma region Rendering

		GE_ADD_INTERNAL_CALL(SpriteRendererComponent_GetColor);
		GE_ADD_INTERNAL_CALL(SpriteRendererComponent_SetColor);

		GE_ADD_INTERNAL_CALL(TextRendererComponent_GetText);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_SetText);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_GetTextColor);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_SetTextColor);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_GetBGColor);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_SetBGColor);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_GetLineHeight);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_SetLineHeight);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_GetLineSpacing);
		GE_ADD_INTERNAL_CALL(TextRendererComponent_SetLineSpacing);

#pragma region GUI
		GE_ADD_INTERNAL_CALL(GUICanvasComponent_GetControlMouse);
		GE_ADD_INTERNAL_CALL(GUICanvasComponent_SetControlMouse);
		GE_ADD_INTERNAL_CALL(GUICanvasComponent_GetShowMouse);
		GE_ADD_INTERNAL_CALL(GUICanvasComponent_SetShowMouse);
		GE_ADD_INTERNAL_CALL(GUICanvasComponent_GetMode);
		GE_ADD_INTERNAL_CALL(GUICanvasComponent_SetMode);

		GE_ADD_INTERNAL_CALL(GUIButtonComponent_GetEnabledColor);
		GE_ADD_INTERNAL_CALL(GUIButtonComponent_SetEnabledColor);
		GE_ADD_INTERNAL_CALL(GUIButtonComponent_GetHoveredColor);
		GE_ADD_INTERNAL_CALL(GUIButtonComponent_SetHoveredColor);

		GE_ADD_INTERNAL_CALL(GUIInputFieldComponent_GetText);
		GE_ADD_INTERNAL_CALL(GUIInputFieldComponent_SetText);
		GE_ADD_INTERNAL_CALL(GUIInputFieldComponent_GetTextColor);
		GE_ADD_INTERNAL_CALL(GUIInputFieldComponent_SetTextColor);
		GE_ADD_INTERNAL_CALL(GUIInputFieldComponent_GetBGColor);
		GE_ADD_INTERNAL_CALL(GUIInputFieldComponent_SetBGColor);
		GE_ADD_INTERNAL_CALL(GUIInputFieldComponent_GetLineHeight);
		GE_ADD_INTERNAL_CALL(GUIInputFieldComponent_SetLineHeight);
		GE_ADD_INTERNAL_CALL(GUIInputFieldComponent_GetLineSpacing);
		GE_ADD_INTERNAL_CALL(GUIInputFieldComponent_SetLineSpacing);

		GE_ADD_INTERNAL_CALL(GUISliderComponent_GetFill);
		GE_ADD_INTERNAL_CALL(GUISliderComponent_SetFill);
#pragma endregion

#pragma endregion

#pragma endregion

	}

	template<typename T>
	void Scripting::RegisterComponent()
	{
		std::string typeName = typeid(T).name();
		size_t namePosition = typeName.find_last_of(':');
		std::string name = typeName.substr(namePosition + 1);
		std::string managedTypeName = fmt::format("GE.{}", name); // Namespace.ComponentName

		MonoType* managedType = mono_reflection_type_from_name(managedTypeName.data(), Scripting::s_Data.CoreAssemblyImage);
		if (!managedType) { GE_CORE_ERROR("Could not Register Component. Component Type not found."); return; }
		s_HasComponentsFuncs[managedType] = [](Ref<Scene> scene, Entity entity) { return scene->HasComponent<T>(entity); };
	}

	void Scripting::RegisterComponents()
	{
		s_HasComponentsFuncs.clear();

		//RegisterComponent<IDComponent>();
		//RegisterComponent<TagComponent>();
		//RegisterComponent<NameComponent>();
		RegisterComponent<TransformComponent>();
		RegisterComponent<ActiveComponent>();
		//RegisterComponent<RelationshipComponent>();
		RegisterComponent<AudioSourceComponent>();
		//RegisterComponent<AudioListenerComponent>();
		//RegisterComponent<RenderComponent>();
		//RegisterComponent<CameraComponent>();
		RegisterComponent<SpriteRendererComponent>();
		//RegisterComponent<CircleRendererComponent>();
		RegisterComponent<TextRendererComponent>();
		RegisterComponent<GUICanvasComponent>();
		RegisterComponent<GUILayoutComponent>();
		// RegisterComponent<GUIMaskComponent>();
		RegisterComponent<GUIImageComponent>();
		RegisterComponent<GUIButtonComponent>();
		RegisterComponent<GUIInputFieldComponent>();
		RegisterComponent<GUISliderComponent>();
		//RegisterComponent<GUICheckboxComponent>();
		// //RegisterComponent<GUIScrollRectComponent>();
		//RegisterComponent<GUIScrollbarComponent>();
		RegisterComponent<Rigidbody2DComponent>();
		//RegisterComponent<BoxCollider2DComponent>();
		//RegisterComponent<CircleCollider2DComponent>();
	}

	void Scripting::OnFileSystemAppAssemblyEvent(const std::string& path, const filewatch::Event changeType)
	{
		if (!s_Data.ReloadPending() && changeType == filewatch::Event::modified)
		{
			s_Data.QueueReload();

			Application::SubmitToMainAppThread([]()
				{	
					ReloadAssembly();
				});
		}
	}
#pragma endregion

#pragma region ScriptInstance
	ScriptInstance::ScriptInstance(UUID scriptHandle, UUID entityID) : m_ScriptHandle(scriptHandle)
	{
		m_Instance = Scripting::InstantiateScriptObject(scriptHandle);

		m_Constructor = Scripting::GetEntityConstructor();
		m_OnCreate = Scripting::GetMethod(scriptHandle, { "OnCreate", 0 });
		m_OnUpdate = Scripting::GetMethod(scriptHandle, { "OnUpdate", 1 });
		m_OnEvent = Scripting::GetMethod(scriptHandle, { "OnEvent", 1 }); // Params : Event(isHandled, Event::Type)
		m_OnKeyPressedEvent = Scripting::GetMethod(scriptHandle, { "OnKeyPressed", 2 }); // Params : Event(isHandled, Event::Type), KeyCode
		m_OnMousePressedEvent = Scripting::GetMethod(scriptHandle, { "OnMousePressed", 2 }); // Params : Event(isHandled, Event::Type), MouseCode
		
		// Invoke Constructor
		if(m_Instance)
		{
			void* param = &entityID;
			Scripting::InvokeMethod(m_Instance, m_Constructor, &param);
		}

	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (m_OnCreate == NULL || m_Instance == NULL)
			return;
		Scripting::InvokeMethod(m_Instance, m_OnCreate, nullptr);
	}

	void ScriptInstance::InvokeOnUpdate(float timestep)
	{
		if (!m_Instance || !m_OnUpdate)
			return;

		void* param = &timestep;
		Scripting::InvokeMethod(m_Instance, m_OnUpdate, &param);
	}

	bool ScriptInstance::InvokeOnEvent(Event& e)
	{
		if (!m_Instance)
			return false;

		switch (e.GetEventType())
		{
		case Event::Type::KeyPressed:
		{
			if (!m_OnKeyPressedEvent)
				break;
			KeyPressedEvent& kpe = *(KeyPressedEvent*)&e;
			uint32_t keycode = kpe.GetKeyCode();
			void* params[] = { &e, &keycode };
			Scripting::InvokeMethod(m_Instance, m_OnKeyPressedEvent, params);
		}
			break;
		case Event::Type::MousePressed:
		{
			if (!m_OnMousePressedEvent)
				break;
			MousePressedEvent& mpe = *(MousePressedEvent*)&e;
			uint32_t mouseButton = mpe.GetButton();
			void* params[] = { &e, &mouseButton };
			Scripting::InvokeMethod(m_Instance, m_OnMousePressedEvent, params);
		}
			break;
		default:
			if (!m_OnEvent)
				break;
			void* params[] = { &e };
			Scripting::InvokeMethod(m_Instance, m_OnEvent, params);
			break;
		}

		// TODO : Get return from method
		return false;
	}

	void ScriptInstance::Internal_GetFieldValue(const std::string& name, void* buffer)
	{
		if (Ref<Script> script = Scripting::GetScript(m_ScriptHandle))
		{
			const auto& fields = script->GetFields();
			if (fields.find(name) == fields.end())
			{
				GE_CORE_WARN("Cannot get Script Instance Field Value. \n\t{0}", name);
				return;
			}

			const ScriptField& field = fields.at(name);
			mono_field_get_value(m_Instance, field.GetData().ClassField, buffer);
		}
	}

	void ScriptInstance::Internal_SetFieldValue(const std::string& name, const void* value)
	{
		if (Ref<Script> script = Scripting::GetScript(m_ScriptHandle))
		{
			const auto& fields = script->GetFields();
			auto instance = fields.find(name);
			if (instance == fields.end())
			{
				GE_CORE_WARN("Cannot set Script Instance Field Value. {0}", name);
				return;
			}

			const ScriptField& field = instance->second;
			mono_field_set_value(m_Instance, field.GetData().ClassField, (void*)value);
		}
	}
#pragma endregion

}