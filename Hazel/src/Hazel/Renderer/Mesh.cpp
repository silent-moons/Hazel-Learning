#include "hzpch.h"

#include <fstream>

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

	UniqueMesh::UniqueMesh()
	{
		InitMesh(std::vector<Vertex>(), std::vector<uint32_t>());
	}

	UniqueMesh::UniqueMesh(
		std::vector<Vertex>& vertices,
		std::vector<uint32_t>& indices)
	{
		InitMesh(vertices, indices);
	}

	void UniqueMesh::Bind() const
	{
		m_VAO->Bind();
	}

	void UniqueMesh::Unbind() const
	{
		m_VAO->Unbind();
	}

	void UniqueMesh::Export(const std::string& path) const
	{
		std::ofstream outputStream(path, std::ios::out | std::ios::binary);

		MeshFileHead meshFileHead;
		meshFileHead.VertexNum = m_Vertices.size();
		meshFileHead.IndexNum = m_Indices.size();
		//写入文件头
		outputStream.write((char*)&meshFileHead, sizeof(meshFileHead));
		//写入顶点数据
		outputStream.write((char*)m_Vertices.data(), m_Vertices.size() * sizeof(Vertex));
		//写入索引数据
		outputStream.write((char*)m_Indices.data(), m_Indices.size() * sizeof(uint32_t));
		outputStream.close();
	}

	void UniqueMesh::InitMesh(
		std::vector<Vertex>& vertices,
		std::vector<uint32_t>& indices)
	{
		m_Vertices = vertices;
		m_Indices = indices;

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

	Ref<BatchMesh> BatchMeshLibrary::s_CubeMesh = nullptr;
	Ref<BatchMesh> BatchMeshLibrary::s_SphereMesh = nullptr;

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
}