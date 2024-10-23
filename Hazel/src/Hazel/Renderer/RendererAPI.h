#pragma once

#include <glm/glm.hpp>

#include "VertexArray.h"

namespace Hazel
{
	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1, DirectX = 2
		};
	public:
		static RendererAPI* Create();

		virtual void Clear() = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;

		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) = 0;

		static API GetAPI() { return s_API; }
		static API SetAPI(API api) { s_API = api; }

	private:
		static API s_API;
	};
}