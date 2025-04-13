#pragma once

namespace Hazel
{
	class IRenderStats
	{
	public:
		uint32_t DrawCalls;
		uint32_t GeometryCount;
	public:
		virtual ~IRenderStats() = default;
		virtual uint32_t GetTotalVertexCount() const = 0;
		virtual uint32_t GetTotalIndexCount() const = 0;
	};
}