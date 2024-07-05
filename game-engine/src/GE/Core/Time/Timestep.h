#pragma once

namespace GE
{
	class Timestep
	{
	public:
		operator float() const { return m_Time; }

		Timestep(float time = 0.0f) : m_Time(time) {};

		float GetSeconds() const { return m_Time; }
		float GetMilliSeconds() const { return m_Time * 1000.0f; }
	private: 
		// In seconds
		float m_Time;
	};

}

