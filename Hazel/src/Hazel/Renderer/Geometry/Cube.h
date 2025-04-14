#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Hazel
{
	struct CubeVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
		int EntityID; // Editor-only
	};

	class Cube
	{
	public:
		static const std::vector<glm::vec4>& GetVertices()
		{
			static std::vector<glm::vec4> vertices
			{
				{-0.5f, -0.5f, -0.5f, 1.0f},
				{ 0.5f, -0.5f, -0.5f, 1.0f},
				{ 0.5f,  0.5f, -0.5f, 1.0f},
				{-0.5f,  0.5f, -0.5f, 1.0f},
				{-0.5f, -0.5f,  0.5f, 1.0f},
				{ 0.5f, -0.5f,  0.5f, 1.0f},
				{ 0.5f,  0.5f,  0.5f, 1.0f},
				{-0.5f,  0.5f,  0.5f, 1.0f},
				{-0.5f, -0.5f, -0.5f, 1.0f},
				{-0.5f, -0.5f,  0.5f, 1.0f},
				{-0.5f,  0.5f,  0.5f, 1.0f},
				{-0.5f,  0.5f, -0.5f, 1.0f},
				{ 0.5f, -0.5f, -0.5f, 1.0f},
				{ 0.5f, -0.5f,  0.5f, 1.0f},
				{ 0.5f,  0.5f,  0.5f, 1.0f},
				{ 0.5f,  0.5f, -0.5f, 1.0f},
				{-0.5f, -0.5f, -0.5f, 1.0f},
				{ 0.5f, -0.5f, -0.5f, 1.0f},
				{ 0.5f, -0.5f,  0.5f, 1.0f},
				{-0.5f, -0.5f,  0.5f, 1.0f},
				{-0.5f,  0.5f, -0.5f, 1.0f},
				{ 0.5f,  0.5f, -0.5f, 1.0f},
				{ 0.5f,  0.5f,  0.5f, 1.0f},
				{-0.5f,  0.5f,  0.5f, 1.0f}
			};
			return vertices;
		}
		static const std::vector<uint32_t>& GetIndices()
		{
			static std::vector<uint32_t> indices
			{
				// 后面
				0, 1, 2,  2, 3, 0,
				// 前面
				4, 5, 6,  6, 7, 4,
				// 左面
				8, 9,10, 10,11, 8,
				// 右面
				12,13,14, 14,15,12,
				// 底面
				16,17,18, 18,19,16,
				// 顶面
				20,21,22, 22,23,20
			};
			return indices;
		}
		static const std::vector<glm::vec2>& GetTextureCoords()
		{
			static std::vector<glm::vec2> textureCoords
			{
				{0.0f, 0.0f},
				{1.0f, 0.0f},
				{1.0f, 1.0f},
				{0.0f, 1.0f},
				{0.0f, 0.0f},
				{1.0f, 0.0f},
				{1.0f, 1.0f},
				{0.0f, 1.0f},
				{0.0f, 0.0f},
				{1.0f, 0.0f},
				{1.0f, 1.0f},
				{0.0f, 1.0f},
				{0.0f, 0.0f},
				{1.0f, 0.0f},
				{1.0f, 1.0f},
				{0.0f, 1.0f},
				{0.0f, 0.0f},
				{1.0f, 0.0f},
				{1.0f, 1.0f},
				{0.0f, 1.0f},
				{0.0f, 0.0f},
				{1.0f, 0.0f},
				{1.0f, 1.0f},
				{0.0f, 1.0f}
			};
			return textureCoords;
		}
		static constexpr int GetVertexCount() { return 24; }
		static constexpr int GetIndexCount() { return 36; }
	};
}