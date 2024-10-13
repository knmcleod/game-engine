#pragma once

#include "GE/Core/Core.h"

#include "GE/Asset/Assets/Scene/Components/Components.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <box2d/b2_body.h>
#include <box2d/b2_world.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>
#pragma warning(pop)

namespace GE
{
	class Physics
	{
	public:
		static b2BodyType Rigidbody2DTypeToBox2DBody(Rigidbody2DComponent::BodyType type)
		{
			switch (type)
			{
			case Rigidbody2DComponent::BodyType::Static:
				return b2_staticBody;
				break;
			case Rigidbody2DComponent::BodyType::Dynamic:
				return b2_dynamicBody;
				break;
			case Rigidbody2DComponent::BodyType::Kinematic:
				return b2_kinematicBody;
				break;
			}
			GE_CORE_ASSERT(false, "Unsupported Rigidbody2D type.");
			return b2_staticBody;
		}

		static Rigidbody2DComponent::BodyType Rigidbody2DTypeFromBox2DBody(b2BodyType type)
		{
			switch (type)
			{
			case b2_staticBody:
				return Rigidbody2DComponent::BodyType::Static;
				break;
			case b2_dynamicBody:
				return Rigidbody2DComponent::BodyType::Dynamic;
				break;
			case b2_kinematicBody:
				return Rigidbody2DComponent::BodyType::Kinematic;
				break;
			}
			GE_CORE_ASSERT(false, "Unsupported Rigidbody2D b2BodyType.");
			return Rigidbody2DComponent::BodyType::Static;
		}
	};
}
