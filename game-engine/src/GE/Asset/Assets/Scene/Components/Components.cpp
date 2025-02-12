#include "GE/GEpch.h"

#include "Components.h"

#include "GE/Audio/AudioManager.h"

#include "GE/Project/Project.h"

#include "GE/Rendering/Renderer/Renderer.h"

namespace GE
{
	static const UUID s_NullUUID = 0;

#pragma region Relationship
	bool RelationshipComponent::HasChild(const UUID& id)
	{
		for (const UUID& child : Children)
		{
			if (child == id)
				return true;
		}
		return false;
	}

	void RelationshipComponent::AddChild(const UUID& id)
	{
		Children.push_back(id);
	}

	void RelationshipComponent::RemoveChild(const UUID& id)
	{
		// Verify parent has child
		if (HasChild(id))
		{
			// Find correct index
			uint64_t index = 0;
			bool verified = false;
			for (const UUID& childID : Children)
			{
				if (childID == id)
				{
					verified = true;
					break;
				}
				else
					index++;
			}

			// If found, erase child from parent at index
			if (verified)
				Children.erase(Children.begin() + index);

		}
	}
	
	const UUID& RelationshipComponent::GetFront() const
	{
		if (Children.empty())
			return s_NullUUID;
		return Children.front();
	}
	const UUID& RelationshipComponent::GetBack() const
	{
		if (Children.empty())
			return s_NullUUID;
		return Children.back();
	}

	const UUID& RelationshipComponent::GetNext(const UUID& id) const
	{	
		uint64_t index = 0;
		if (!GetChildIndex(id, index) || Children.size() <= index + 1)
		{
			GE_CORE_WARN("RelationshipComponent::GetNext(const UUID&) - Next Child does not exist.");
			return GetFront();
		}
		return Children.at(index + 1);
	}
	const UUID& RelationshipComponent::GetPrevious(const UUID& id) const
	{
		uint64_t index = 0;
		if (!GetChildIndex(id, index) || index <= 0 || Children.size() < index - 1)
		{
			GE_CORE_WARN("RelationshipComponent::GetPrevious(const UUID&) - Previous Child does not exist.");
			return GetBack();
		}
		return Children.at(index - 1);
	}

	bool RelationshipComponent::GetChildIndex(const UUID& id, uint64_t& index) const
	{
		bool found = false;
		for (const auto& uuid : Children)
		{
			if (uuid == id)
			{
				found = true;
				break;
			}
			index++;
		}
		return found;
	}
#pragma endregion


#pragma region Transform
	const glm::mat4 TransformComponent::GetTransform(const glm::vec3& translationOffset, const glm::vec3& rotationOffset, const glm::vec3& scaleOffset, const glm::vec3& pivotOffset) const
	{
		glm::mat4 identityMat4 = glm::mat4(1.0f);

		glm::mat4 rotation = glm::rotate(identityMat4, glm::radians(Rotation.x + rotationOffset.x), { 1, 0, 0 })
			* glm::rotate(identityMat4, glm::radians(Rotation.y + rotationOffset.y), { 0, 1, 0 })
			* glm::rotate(identityMat4, glm::radians(Rotation.z + rotationOffset.z), { 0, 0, 1 });

		return glm::translate(identityMat4, (Translation + (PivotOffset + pivotOffset)) + translationOffset)
			* rotation * glm::scale(identityMat4, Scale * scaleOffset);
	}

	const std::pair<glm::vec2, glm::vec2> TransformComponent::GetBounds() const
	{
		std::pair<glm::vec2, glm::vec2> ret = std::pair<glm::vec2, glm::vec2>();

		// Offset by Pivot
		ret.first.x -= 0.5f;
		ret.first.y -= 0.5f;

		ret.second.x += 0.5f;
		ret.second.y += 0.5f;

		// Offset by Scale
		ret.first.x *= Scale.x;
		ret.first.y *= Scale.y;

		ret.second.x *= Scale.x;
		ret.second.y *= Scale.y;

		return ret;
	}

	const glm::vec3 TransformComponent::GetOffsetTranslation() const
	{
		return Translation + PivotOffset;
	}
	void TransformComponent::SetPivot(const Pivot& pivot)
	{
		switch (pivot)
		{
		case Pivot::Center:
			PivotOffset = glm::vec3(0.0f);
			break;
		case Pivot::LowerLeft:
			PivotOffset = glm::vec3(-0.5f, -0.5f, 0.0f);
			break;
		case Pivot::TopLeft:
			PivotOffset = glm::vec3(-0.5f, 0.5f, 0.0f);
			break;
		case Pivot::TopRight:
			PivotOffset = glm::vec3(0.5f, 0.5f, 0.0f);
			break;
		case Pivot::LowerRight:
			PivotOffset = glm::vec3(0.5f, -0.5f, 0.0f);
			break;
		case Pivot::MiddleRight:
			PivotOffset = glm::vec3(0.5f, 0.0f, 0.0f);
			break;
		case Pivot::TopMiddle:
			PivotOffset = glm::vec3(0.0f, 0.5f, 0.0f);
			break;
		case Pivot::MiddleLeft:
			PivotOffset = glm::vec3(-0.5f, 0.0f, 0.0f);
			break;
		case Pivot::BottomMiddle:
			PivotOffset = glm::vec3(0.0f, -0.5f, 0.0f);
			break;
		default:
			PivotOffset = glm::vec3(0.0f);
			PivotEnum = Pivot::Center;
			GE_CORE_WARN("TransformComponet::SetPivot(const Pivot&) - Could not set PivotOffset.\n\tUnknown Pivot.");
			return;
		}
		PivotEnum = pivot;
	}
#pragma endregion

#pragma region AudioSource
	void AudioSourceComponent::Play(const glm::vec3& position, const glm::vec3& velocity) const
	{
		Ref<Audio> audio = Project::GetAsset<Audio>(AssetHandle);
		if (!audio || audio->GetType() != Asset::Type::Audio)
			return;

		AudioManager::Play(ID, Loop, Pitch, Gain, position, velocity, audio);
	}
	void AudioSourceComponent::Pause() const
	{
		AudioManager::Pause(ID);
	}

	void AudioSourceComponent::Stop() const
	{
		AudioManager::Stop(ID);
	}

	bool AudioSourceComponent::IsPlaying() const
	{
		return AudioManager::IsSourcePlaying(ID);
	}

#pragma endregion

	AudioListenerComponent::AudioListenerComponent()
	{
		Device = AudioManager::GetDevice();
	}

	const glm::vec2 GUILayoutComponent::GetEntityOffset()
	{
		switch (Mode)
		{
		case GE::LayoutMode::Horizontal:
			return (ChildSize + ChildPadding) * glm::vec2(1.0f, 0.0f);
			break;
		case GE::LayoutMode::Vertical:
			return (ChildSize + ChildPadding) * glm::vec2(0.0f, -1.0f);
			break;
		default:
			break;
		}
		return glm::vec2(0.0f);
	}

}