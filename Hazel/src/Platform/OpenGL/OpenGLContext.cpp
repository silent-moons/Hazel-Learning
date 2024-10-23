#include "hzpch.h"
#include "Hazel/Core.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Hazel 
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		HZ_CORE_ASSERT(windowHandle, "Window handle is null!");
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		HZ_CORE_ASSERT(status, "Failed to initialize Glad!");

		HZ_CORE_INFO("OpenGL info:");
		HZ_CORE_INFO("	Vendor£º{0}", (const char*)glGetString(GL_VENDOR));
		HZ_CORE_INFO("	Renderer£º{0}", (const char*)glGetString(GL_RENDERER));
		HZ_CORE_INFO("	Version£º{0}", (const char*)glGetString(GL_VERSION));

		HZ_CORE_ASSERT((GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5)), "Hazel requires at least OpenGL version 4.5!");
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}

}