#include "hzpch.h"

#include "RendererAPI.h"
#include "Buffer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Hazel 
{
	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size) 
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None: HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported! ")
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLVertexBuffer(vertices, size);
		case RendererAPI::API::DirectX: HZ_CORE_ASSERT(false, "RendererAPI::DirectX is currently not supported! ")
			return nullptr;
		}

		HZ_CORE_ASSERT(false, "Unknown Renderer API!")
			return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count) 
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:	HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported! ")
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLIndexBuffer(indices, count);
		case RendererAPI::API::DirectX:	HZ_CORE_ASSERT(false, "RendererAPI::DirectX is currently not supported! ")
			return nullptr;
		}
		HZ_CORE_ASSERT(false, "Unknown Renderer API!")
			return nullptr;
	}
}