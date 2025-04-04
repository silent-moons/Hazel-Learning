#pragma once

#include "Hazel/Core/Timestep.h"
#include "Hazel/Events/Event.h"

namespace Hazel 
{
	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnEvent(Event& event) {}
		virtual void OnImGuiRender() {}

		const std::string& GetName() const { return m_Name; }
	protected:
		std::string m_Name;
	};

}