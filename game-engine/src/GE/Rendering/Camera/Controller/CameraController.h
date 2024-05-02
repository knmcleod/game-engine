#pragma once

#include "GE/Asset/Assets/Scene/Scene.h"

namespace GE
{
	class CameraController : public ScriptableEntity
	{
	public:
		virtual void OnCreate() override;

		virtual void OnDestroy() override;

		virtual void OnUpdate(Timestep timestep) override;
	};
}