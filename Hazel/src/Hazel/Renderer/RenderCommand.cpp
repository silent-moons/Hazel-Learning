#include "hzpch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Hazel 
{
	RendererAPI* RendererAPI::Create() 
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None: HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported! ")
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLRendererAPI();
		case RendererAPI::API::DirectX: HZ_CORE_ASSERT(false, "RendererAPI::DirectX is currently not supported! ")
			return nullptr;
		}
		HZ_CORE_ASSERT(false, "Unknown Renderer API!")
			return nullptr;
	}

	RendererAPI* RenderCommand::s_RendererAPI = RendererAPI::Create();
}