#pragma once

#include "Hazel/Renderer/RendererAPI.h"

namespace Hazel 
{
	class RenderCommand
	{
	public:
		static void Init()
		{
			s_RendererAPI->Init();
		}

		static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		static void Clear()
		{
			s_RendererAPI->Clear();
		}

		static void DrawIndexed(const Ref<VertexArray>& vertexArray) //GL_STATIC_DRAW
		{
			s_RendererAPI->DrawIndexed(vertexArray);
		}

		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) //GL_DYNAMIC_DRAW	//GL_DYNAMIC_DRAW(with batch rendering)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount);
		}

	private:
		static Scope<RendererAPI> s_RendererAPI;
	};

}