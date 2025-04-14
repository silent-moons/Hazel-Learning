#pragma once

namespace Hazel
{
	struct RenderStats
	{
	public:
		uint32_t DrawCalls;
		uint32_t GeometryCount;
		uint32_t VertexCount;
		uint32_t IndexCount;
	};
}