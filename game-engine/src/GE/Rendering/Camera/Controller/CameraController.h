#pragma once
#include "GE/Core/Time/Time.h"
#include "GE/Scene/Entity/ScriptableEntity.h"

namespace GE
{
	class CameraController : public ScriptableEntity
	{
	public:
		virtual void OnCreate() {}

		virtual void OnDestroy() {}

		virtual void OnUpdate(Timestep timestep);
	};
}