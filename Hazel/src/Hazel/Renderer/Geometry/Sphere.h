#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Hazel
{
	struct SphereVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
		int EntityID;
	};

	class Sphere
	{
	public:
		static const std::vector<glm::vec4>& GetVertices()
		{
			static std::vector<glm::vec4> vertices;
			constexpr int numLatLines = 32;
			constexpr int numLongLines = 64;
			constexpr float radius = 1;
			if (vertices.empty())
			{
				for (unsigned int i = 0; i <= numLatLines; ++i)
				{
					float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / numLatLines; // �� ��/2 �� -��/2
					float xy = radius * cosf(stackAngle);            // ��ǰ���Բ�İ뾶
					float z = radius * sinf(stackAngle);             // ��ǰ��� z �߶�

					for (unsigned int j = 0; j <= numLongLines; ++j)
					{
						float sectorAngle = j * 2 * glm::pi<float>() / numLongLines; // �� 0 �� 2��

						float x = xy * cosf(sectorAngle);
						float y = xy * sinf(sectorAngle);

						vertices.emplace_back(x, y, z, 1.0f);
					}
				}
			}
			return vertices;
		}
		static const std::vector<uint32_t>& GetIndices()
		{
			static std::vector<uint32_t> indices;
			constexpr int numLatLines = 32;
			constexpr int numLongLines = 64;
			if (indices.empty())
			{
				for (int i = 0; i < numLatLines; i++)
				{
					for (int j = 0; j < numLongLines; j++)
					{
						int first = i * (numLongLines + 1) + j;
						int second = first + numLongLines + 1;

						// ���������ι����ı���
						indices.push_back(first);
						indices.push_back(second);
						indices.push_back(first + 1);
						indices.push_back(second);
						indices.push_back(second + 1);
						indices.push_back(first + 1);
					}
				}
			}
			return indices;
		}
		static const std::vector<glm::vec2>& GetTextureCoords()
		{
			static std::vector<glm::vec2> textureCoords;
			constexpr int numLatLines = 32;
			constexpr int numLongLines = 64;
			if (textureCoords.empty())
			{
				for (int i = 0; i <= numLatLines; i++)
				{
					for (int j = 0; j <= numLongLines; j++)
					{
						float u = (float)j / numLongLines;
						float v = (float)i / numLatLines;
						textureCoords.emplace_back(u, v);
					}
				}
			}
			return textureCoords;
		}
	};
}