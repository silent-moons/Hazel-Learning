#include "hzpch.h"

#include "Mesh.h"
#include "Hazel/Renderer/Geometry/Geometry.h"
#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Renderer/Texture.h" // temporary

namespace Hazel
{
	void BatchMesh::LoadBaseGeometry(
		const std::vector<glm::vec3>& vertices, 
		const std::vector<uint32_t>& indices, 
		const std::vector<glm::vec2>& textureCoords)
	{
		m_VerticesPositions = vertices;
		m_Indices = indices;
		m_TextureCoords = textureCoords;
		m_MeshType = MeshType::StaticBatchable;
	}

	UniqueMesh::UniqueMesh(
		std::vector<Vertex>& vertices,
		std::vector<uint32_t>& indices,
		std::vector<Ref<Texture2D>>& textures)
	{
		m_Vertices = vertices;
		m_Indices = indices;
		m_Textures = textures;
		m_MeshType = MeshType::StaticUnique;
		m_VAO = VertexArray::Create();
		m_VBO = VertexBuffer::Create(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(Vertex)));
		m_IBO = IndexBuffer::Create(indices.data(), static_cast<uint32_t>(indices.size()));
		m_VAO->Bind();
		BufferLayout layout =
		{
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float3, "a_Tangent" },
			{ ShaderDataType::Float3, "a_Bitangent" }
		};
		m_VBO->SetLayout(layout);
		m_VAO->AddVertexBuffer(m_VBO);
		m_VAO->SetIndexBuffer(m_IBO);
	}

	void UniqueMesh::Bind() const
	{
		m_VAO->Bind();
	}

	void UniqueMesh::Unbind() const
	{
		m_VAO->Unbind();
	}

	Ref<BatchMesh> BatchMeshLibrary::s_CubeMesh = nullptr;
	Ref<BatchMesh> BatchMeshLibrary::s_SphereMesh = nullptr;
	Ref<Mesh> BatchMeshLibrary::s_TempMesh = nullptr;

	Ref<BatchMesh> BatchMeshLibrary::GetCubeMesh()
	{
		if (!s_CubeMesh)
		{
			s_CubeMesh = CreateRef<BatchMesh>();
			s_CubeMesh->LoadBaseGeometry(
				Cube::GetVertices(),
				Cube::GetIndices(),
				Cube::GetTextureCoords()
			);
		}
		return s_CubeMesh;
	}

	Ref<BatchMesh> BatchMeshLibrary::GetSphereMesh()
	{
		if (!s_SphereMesh)
		{
			s_SphereMesh = CreateRef<BatchMesh>();
			s_SphereMesh->LoadBaseGeometry(
				Sphere::GetVertices(),
				Sphere::GetIndices(),
				Sphere::GetTextureCoords()
			);
		}
		return s_SphereMesh;
	}

	Ref<Mesh> BatchMeshLibrary::GetTempMesh()
	{
		if (!s_TempMesh)
		{
			std::vector<Vertex> vertices
			{
				{ {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
				{ { 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
				{ { 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
				{ {-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }
			};
			std::vector<uint32_t> indices{ 0, 1, 2, 2, 3, 0 };
			std::vector<Ref<Texture2D>> textures{ Texture2D::Create(1, 1) };
			uint32_t whiteTextureData = 0xffffffff;
			for (size_t i = 0; i < textures.size(); i++)
				textures[i]->SetData(&whiteTextureData, sizeof(uint32_t));
			s_TempMesh = CreateRef<UniqueMesh>(vertices, indices, textures);
		}
		return s_TempMesh;
	}
}