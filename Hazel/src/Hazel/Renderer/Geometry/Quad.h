#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Hazel
{
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
		int EntityID; // Editor-only
	};

	class Quad
	{
	public:
		static const std::vector<glm::vec4>& GetVertices()
		{
			static std::vector<glm::vec4> vertices
			{
				{ -0.5f, -0.5f, 0.0f, 1.0f },
				{  0.5f, -0.5f, 0.0f, 1.0f },
				{  0.5f,  0.5f, 0.0f, 1.0f },
				{ -0.5f,  0.5f, 0.0f, 1.0f }
			};
			return vertices;
		}
		static const std::vector<uint32_t>& GetIndices()
		{
			static std::vector<uint32_t> indices{ 0, 1, 2, 2, 3, 0 };
			return indices;
		}
		static const std::vector<glm::vec2>& GetTextureCoords()
		{
			static std::vector<glm::vec2> textureCoords
			{
				{ 0.0f, 0.0f },
				{ 1.0f, 0.0f },
				{ 1.0f, 1.0f },
				{ 0.0f, 1.0f }
			};
			return textureCoords;
		}
	};
}