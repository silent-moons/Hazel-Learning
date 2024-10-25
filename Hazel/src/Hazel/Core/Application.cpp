#include "hzpch.h"
#include "Application.h"

#include "Hazel/Events/Event.h"
#include "Hazel/Core/Log.h"
#include "Hazel/Core/Input.h"
#include "Hazel/Renderer/Renderer.h"

#include <GLFW/glfw3.h>
#include <stb_image.h>

namespace Hazel
{
	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name)
	{
		stbi_set_flip_vertically_on_load(1);

		HZ_CORE_ASSERT(!s_Instance, "Application already exists! (The class Application is a Singleton, it just support one instance!)");
		s_Instance = this;

		m_Window = Window::Create(WindowProps(name));
		m_Window->SetEventCallback(HZ_BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(true);
		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		Renderer::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(HZ_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(HZ_BIND_EVENT_FN(Application::OnWindowResize));
		//HZ_CORE_TRACE("{0}", e);

		//图层的事件处理是反向的（从尾到头）
		for (auto iter = m_LayerStack.rbegin(); iter != m_LayerStack.rend(); ++iter)
		{
			if (e.Handled) //如果在OnEvent中成功进行处理并将Handled变为true，则跳出循环
				break;
			
			(*iter)->OnEvent(e); //从最后一个迭代器所指的元素开始，逐个逆向相应事件
		}
	}

	void Application::Run()
	{
		while (m_Running)
		{
			float time = (float)glfwGetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			for (Layer* layer : m_LayerStack)
			{
				layer->OnUpdate(timestep);
			}
				
			//glm::vec2 a = Input::GetMousePosition();
			//HZ_CORE_TRACE("{0},{1}", a.x, a.y);

			m_ImGuiLayer->Begin();

			if (!m_Minimized)
			{
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
			}

			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& event)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& event)
	{
		if (event.GetWidth() == 0 || event.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());

		return false;
	}
}
