#pragma once

#include "Core.h"

#include "Window.h"
#include "LayerStack.h"

#include "Hazel/ImGui/ImGuiLayer.h"
#include "Hazel/Events/ApplicationEvent.h"

namespace Hazel
{
	class Application
	{
	public:
		Application();
		virtual ~Application() = default;

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		Window& GetWindow() { return *m_Window; }
		static Application& Get() { return *s_Instance; }

	private:
		bool OnWindowClose(WindowCloseEvent& event);
		bool OnWindowResize(WindowResizeEvent& event);

	private:
		ImGuiLayer* m_ImGuiLayer;
		std::unique_ptr<Window> m_Window;
		LayerStack m_LayerStack;

		float m_LastFrameTime = 0.0f;

		bool m_Running = true;
		bool m_Minimized = false;
	private:
		static Application* s_Instance;
	};

	Application* CreateApplication(); // 让客户端使用
}