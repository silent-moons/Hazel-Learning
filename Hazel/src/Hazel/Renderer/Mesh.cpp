#include "hzpch.h"

#include "Mesh.h"
#include "Hazel/Renderer/Geometry/Geometry.h"

namespace Hazel
{
	void Mesh::LoadBaseGeometry(
		const std::vector<glm::vec4>& vertices, 
		const std::vector<uint32_t>& indices, 
		const std::vector<glm::vec2>& textureCoords)
	{
		m_Vertices = vertices;
		m_Indices = indices;
		m_TextureCoords = textureCoords;
		m_MeshType = MeshType::StaticBatchable;
	}

	Ref<Mesh> MeshLibrary::s_CubeMesh = nullptr;
	Ref<Mesh> MeshLibrary::s_SphereMesh = nullptr;

	Ref<Mesh> MeshLibrary::GetCubeMesh() 
	{
		if (!s_CubeMesh)
		{
			s_CubeMesh = CreateRef<Mesh>();
			s_CubeMesh->LoadBaseGeometry(
				Cube::GetVertices(),
				Cube::GetIndices(),
				Cube::GetTextureCoords()
			);
		}
		return s_CubeMesh;
	}

	Ref<Mesh> MeshLibrary::GetSphereMesh() 
	{
		if (!s_SphereMesh)
		{
			s_SphereMesh = CreateRef<Mesh>();
			s_SphereMesh->LoadBaseGeometry(
				Sphere::GetVertices(),
				Sphere::GetIndices(),
				Sphere::GetTextureCoords()
			);
		}
		return s_SphereMesh;
	}
}