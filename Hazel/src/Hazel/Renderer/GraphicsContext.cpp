#include "hzpch.h"
#include "GraphicsContext.h"

#include "Hazel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLContext.h"

namespace Hazel 
{
	Scope<GraphicsContext> GraphicsContext::Create(void* window)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(window));
		case RendererAPI::API::DirectX: HZ_CORE_ASSERT(false, "RendererAPI::DirectX is currently not supported! ");
			return nullptr;
		}
		HZ_CORE_ASSERT(false, "Unknown RendererAPI! ");
		return nullptr;
	}
}