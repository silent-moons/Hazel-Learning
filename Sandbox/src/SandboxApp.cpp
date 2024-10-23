#include "Hazel.h"

#include "imgui/imgui.h"

class ExampleLayer : public Hazel::Layer
{
public:
	ExampleLayer() :Layer("Example layer") 
	{ 
	}

	void OnUpdate() override
	{
		if (Hazel::Input::IsKeyPressed(Hazel::Key::Tab))
		{
			HZ_TRACE("tab is pressed");
		}
	}

	void OnImGuiRender()
	{
		ImGui::Begin("Test");
		ImGui::Text("Hello world");
		ImGui::End();
	}

	void OnEvent(Hazel::Event& event) override
	{
		if (event.GetEventType() == Hazel::EventType::KeyPressed)
		{
			Hazel::KeyPressedEvent& e = (Hazel::KeyPressedEvent&)event;
			HZ_TRACE("{0} is pressed", (char)e.GetKeyCode());
		}
	}
};

class Sandbox : public Hazel::Application
{
public:
	Sandbox() 
	{ 
		PushLayer(new ExampleLayer());
	}
	~Sandbox() {}

private:

};

Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}