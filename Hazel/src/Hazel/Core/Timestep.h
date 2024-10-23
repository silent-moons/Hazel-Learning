#pragma once

namespace Hazel 
{
	class Timestep
	{
	public:
		Timestep(float time) : m_Time(time) {}

		operator float() const { return m_Time; } // 允许将该类对象隐式转换为float类型

		float GetSeconds() const { return m_Time; }
		float GetMilliseconds() const { return m_Time * 1000.0f; }

	private:
		float m_Time = 0.0f;
	};
}