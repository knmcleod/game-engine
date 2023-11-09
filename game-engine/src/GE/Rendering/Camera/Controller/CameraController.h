#pragma once
#include "GE/Scene/Entity/ScriptableEntity.h"
#include "GE/Core/Time/Time.h"

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