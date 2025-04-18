#pragma once

#include "Hazel/Core/Window.h"
#include "Hazel/Core/LayerStack.h"

#include "Hazel/ImGui/ImGuiLayer.h"
#include "Hazel/Events/ApplicationEvent.h"

int main(int argc, char** argv);

namespace Hazel
{
	class Application
	{
	public:
		Application(const std::string& name = "Hazel App");
		virtual ~Application();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		Window& GetWindow() { return *m_Window; }
		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
		void Close();

		static Application& Get() { return *s_Instance; }

	private:
		void Run();

		bool OnWindowClose(WindowCloseEvent& event);
		bool OnWindowResize(WindowResizeEvent& event);

	private:
		ImGuiLayer* m_ImGuiLayer;
		Scope<Window> m_Window;
		LayerStack m_LayerStack;

		float m_LastFrameTime = 0.0f;

		bool m_Running = true;
		bool m_Minimized = false;
	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	Application* CreateApplication(); // �ÿͻ���ʹ��
}