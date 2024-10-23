#pragma once

#include "Core.h"
#include "Window.h"
#include "LayerStack.h"

#include "Hazel/ImGui/ImGuiLayer.h"
#include "Hazel/Events/ApplicationEvent.h"

#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Buffer.h"

namespace Hazel
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		Window& GetWindow() { return *m_Window; }
		static Application& Get() { return *s_Instance; }

	private:
		bool OnWindowClose(WindowCloseEvent& event);

		ImGuiLayer* m_ImGuiLayer;
		std::unique_ptr<Window> m_Window;
		LayerStack m_LayerStack;

		std::unique_ptr<Shader> m_Shader;
		std::shared_ptr<VertexArray> m_VertexArray;

		std::shared_ptr<VertexArray> m_SquareVA;
		std::shared_ptr<Shader> m_SquareShader;

		bool m_Running = true;
	private:
		static Application* s_Instance;
	};

	Application* CreateApplication(); // 让客户端使用
}