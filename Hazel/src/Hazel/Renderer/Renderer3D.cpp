#include "hzpch.h"
#include "Renderer3D.h"

#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Material.h"
#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/RenderStats.h"
#include "Hazel/Renderer/Geometry/Cube.h"
#include "Hazel/Renderer/Geometry/Sphere.h"
#include "Hazel/Renderer/Geometry/Geometry.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
namespace Hazel
{
	struct RendererBatchData
	{
		Ref<VertexArray> VAO;
		Ref<VertexBuffer> VBO;
		Ref<IndexBuffer> IBO;

		uint32_t IndexCount = 0;
		BatchVertex* VertexBufferBase = nullptr; // 顶点指针起始
		BatchVertex* VertexBufferPtr = nullptr; // 用于移动的顶点指针
	};

	static std::unordered_map<MeshFilterComponent::GeometryType, RendererBatchData> s_BatchDataMap;
	struct Renderer3DData
	{
		static constexpr uint32_t MaxCubes = 100;
		static constexpr uint32_t MaxCubeVertices = MaxCubes * Cube::GetVertexCount();
		static constexpr uint32_t MaxCubeIndices = MaxCubes * Cube::GetIndexCount();
		static constexpr uint32_t MaxSpheres = 50;
		static constexpr uint32_t MaxSphereVertices = MaxSpheres * Sphere::GetVertexCount();
		static constexpr uint32_t MaxSphereIndices = MaxSpheres * Sphere::GetIndexCount();
		static constexpr uint32_t MaxTextureSlots = 32;

		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;

		RenderStats Stats;
	};
	static Renderer3DData s_Data;

	void Renderer3D::Init()
	{
		// 立方体合批
		RendererBatchData cubeData;
		cubeData.VBO = VertexBuffer::Create(s_Data.MaxCubeVertices * sizeof(BatchVertex));
		BufferLayout cubeLayout =
		{
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float4, "a_Color"},
			{ShaderDataType::Float2, "a_TexCoord"},
			{ShaderDataType::Float, "a_TexIndex"},
			{ShaderDataType::Float, "a_TilingFactor"},
			{ShaderDataType::Int, "a_EntityID"}
		};
		cubeData.VBO->SetLayout(cubeLayout);

		// 制作EBO数组
		uint32_t* cubeIndices = new uint32_t[s_Data.MaxCubeIndices];
		std::vector<uint32_t> indices = Cube::GetIndices();
		for (uint32_t i = 0; i < s_Data.MaxCubeIndices; i += Cube::GetIndexCount())
		{
			memcpy(cubeIndices + i, indices.data(), indices.size() * sizeof(uint32_t));
			for (int j = 0; j < Cube::GetIndexCount(); j++)
				indices[j] += Cube::GetVertexCount();
		}
		cubeData.IBO = IndexBuffer::Create(cubeIndices, s_Data.MaxCubeIndices);
		delete[] cubeIndices;

		cubeData.VAO = VertexArray::Create();
		cubeData.VAO->SetIndexBuffer(cubeData.IBO);
		cubeData.VAO->AddVertexBuffer(cubeData.VBO);
		cubeData.VAO->Unbind();
		cubeData.VertexBufferBase = new BatchVertex[s_Data.MaxCubeVertices]; //保存指针初始位置
		s_BatchDataMap[MeshFilterComponent::GeometryType::Cube] = cubeData;

		// 球体合批
		RendererBatchData sphereData;
		sphereData.VBO = VertexBuffer::Create(s_Data.MaxSphereVertices * sizeof(BatchVertex));
		sphereData.VBO->SetLayout(cubeLayout);

		// 制作EBO数组
		uint32_t* sphereIndices = new uint32_t[s_Data.MaxSphereIndices];
		indices = Sphere::GetIndices();
		for (uint32_t i = 0; i < s_Data.MaxSphereIndices; i += Sphere::GetIndexCount())
		{
			memcpy(sphereIndices + i, indices.data(), indices.size() * sizeof(uint32_t));
			for (int j = 0; j < Sphere::GetIndexCount(); j++)
				indices[j] += Sphere::GetVertexCount();
		}
		sphereData.IBO = IndexBuffer::Create(sphereIndices, s_Data.MaxSphereIndices);
		delete[] sphereIndices;

		sphereData.VAO = VertexArray::Create();
		sphereData.VAO->SetIndexBuffer(sphereData.IBO);
		sphereData.VAO->AddVertexBuffer(sphereData.VBO);
		sphereData.VAO->Unbind();
		sphereData.VertexBufferBase = new BatchVertex[s_Data.MaxSphereVertices]; //保存指针初始位置
		s_BatchDataMap[MeshFilterComponent::GeometryType::Sphere] = sphereData;

		// 纹理采样器
		int32_t samplers[s_Data.MaxTextureSlots];
		for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
			samplers[i] = i;

		// Shader，
		if (ShaderLibrary::Exists("UnlitBatch"))
			s_Data.TextureShader = ShaderLibrary::Get("UnlitBatch");
		else
			s_Data.TextureShader = ShaderLibrary::Load("assets/shaders/UnlitBatch.glsl");
		//上传所有采样器到对应纹理单元
		s_Data.TextureShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);
		s_Data.TextureShader->Unbind();

		// Texture
		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
		s_Data.TextureSlots[0] = s_Data.WhiteTexture;
	}

	void Renderer3D::Shutdown()
	{
		for (auto& [_, data] : s_BatchDataMap)
			delete[] data.VertexBufferBase;
	}

	void Renderer3D::BeginBatch(const Camera& camera, const glm::mat4& transform)
	{
		glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);
		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_ViewProjection", viewProj);
		StartBatch();
	}

	void Renderer3D::BeginBatch(const EditorCamera& camera)
	{
		glm::mat4 viewProj = camera.GetViewProjection();
		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_ViewProjection", viewProj);
		StartBatch();
	}

	void Renderer3D::EndBatch()
	{
		Flush();
		s_Data.TextureShader->Unbind();
	}

	void Renderer3D::StartBatch()
	{
		for (auto& [_, data] : s_BatchDataMap)
		{
			data.IndexCount = 0; //每结束（刷新）一次批渲染，需要绘制的索引数要从零重新开始
			data.VertexBufferPtr = data.VertexBufferBase; // 顶点数据指针置于起点
		}
		s_Data.TextureSlotIndex = 1;
	}

	void Renderer3D::Flush()
	{
		for (auto& [_, data] : s_BatchDataMap)
		{
			if (data.IndexCount == 0)
				continue;
			// Size 等于后端指针减去前端
			uint32_t dataSize = uint32_t((uint8_t*)data.VertexBufferPtr - (uint8_t*)data.VertexBufferBase);
			// 将数据送往GPU
			data.VBO->SetData(data.VertexBufferBase, dataSize);
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
				s_Data.TextureSlots[i]->Bind(i);
			RenderCommand::DrawIndexed(data.VAO, data.IndexCount);
			s_Data.Stats.DrawCalls++;
		}
	}

	void Renderer3D::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Renderer3D::DrawBatch(const glm::mat4& transform, MeshFilterComponent::GeometryType type, const Ref<Mesh>& mesh, const glm::vec4& color, int entityID)
	{
		const float textureIndex = 0.0f; // White Texture
		const float tilingFactor = 1.0f;

		if (s_BatchDataMap.find(type) == s_BatchDataMap.end())
			return;
		RendererBatchData& batchData = s_BatchDataMap[type];
		if (batchData.IndexCount >= Renderer3DData::MaxSphereIndices)
			NextBatch();

		for (size_t i = 0; i < mesh->GetVertexCount(); i++)
		{
			batchData.VertexBufferPtr->Position = transform * glm::vec4(mesh->GetVerticesPositions()[i], 1.0f);
			batchData.VertexBufferPtr->Color = color;
			batchData.VertexBufferPtr->TexCoord = mesh->GetTextureCoords()[i];
			batchData.VertexBufferPtr->TexIndex = textureIndex;
			batchData.VertexBufferPtr->TilingFactor = tilingFactor;
			batchData.VertexBufferPtr->EntityID = entityID;
			batchData.VertexBufferPtr++;
		}

		batchData.IndexCount += mesh->GetIndexCount();
		s_Data.Stats.VertexCount += mesh->GetVertexCount();
		s_Data.Stats.IndexCount += mesh->GetIndexCount();
		s_Data.Stats.GeometryCount++;
	}

	void Renderer3D::DrawBatch(const glm::mat4& transform, MeshFilterComponent::GeometryType type, const Ref<Mesh>& mesh, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor, int entityID)
	{
		if (s_BatchDataMap.find(type) == s_BatchDataMap.end())
			return;
		RendererBatchData& batchData = s_BatchDataMap[type];
		if (batchData.IndexCount >= Renderer3DData::MaxSphereIndices)
			NextBatch();

		// 遍历纹理，查看现有纹理是否已经存入
		float textureIndex = 0.0f;

		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		// 若未命中，则将纹理存入最新的位置
		if (textureIndex == 0.0f)
		{
			if (s_Data.TextureSlotIndex >= Renderer3DData::MaxTextureSlots)
				NextBatch();

			textureIndex = (float)s_Data.TextureSlotIndex; // 存入后自增一次
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		for (size_t i = 0; i < mesh->GetVertexCount(); i++)
		{
			batchData.VertexBufferPtr->Position = transform * glm::vec4(mesh->GetVerticesPositions()[i], 1.0f);
			batchData.VertexBufferPtr->Color = tintColor;
			batchData.VertexBufferPtr->TexCoord = mesh->GetTextureCoords()[i];
			batchData.VertexBufferPtr->TexIndex = textureIndex;
			batchData.VertexBufferPtr->TilingFactor = tilingFactor;
			batchData.VertexBufferPtr->EntityID = entityID;
			batchData.VertexBufferPtr++;
		}

		batchData.IndexCount += mesh->GetIndexCount();
		s_Data.Stats.VertexCount += mesh->GetVertexCount();
		s_Data.Stats.IndexCount += mesh->GetIndexCount();
		s_Data.Stats.GeometryCount++;
	}

	void Renderer3D::DrawUnique(const glm::mat4& transform, const Ref<Mesh>& mesh, const Ref<Material>& mat, int entityID)
	{
		for (size_t i = 0; i < mat->GetTextures().size(); i++)
		{
			mat->GetTextures()[i]->Bind(i);
			mat->GetShader()->SetInt("u_TextureDiffuse1", i);
		}
		mat->GetShader()->SetInt("u_EntityID", entityID);
		RenderCommand::DrawIndexed(mesh->GetVAO(), mesh->GetIndexCount());
		s_Data.Stats.VertexCount += mesh->GetVertexCount();
		s_Data.Stats.IndexCount += mesh->GetIndexCount();
		s_Data.Stats.DrawCalls++;
		s_Data.Stats.GeometryCount++;
	}

	void Renderer3D::DrawMesh(
		const Camera& camera,
		const glm::mat4& cameraTrans, 
		const glm::mat4& transform, 
		MeshFilterComponent& mfc, 
		MeshRendererComponent& mrc, 
		int entityID)
	{
		if (!mrc.Mat)
			return;
		std::unordered_map<std::string, std::pair<ShaderPropertyType, glm::vec4>>& attribInfo = mrc.Mat->AttribInfo;
		if (mfc.GType == MeshFilterComponent::GeometryType::Custom)
		{
			glm::mat4 viewProj = camera.GetViewProjection() * glm::inverse(cameraTrans);
			mrc.Mat->GetShader()->Bind();
			mrc.Mat->GetShader()->SetMat4("u_ViewProjection", viewProj);
			mrc.Mat->GetShader()->SetMat4("u_Model", transform);
			mrc.Mat->GetShader()->SetFloat4("u_Color", attribInfo.at("Color").second);
			DrawUnique(transform, mfc.MeshObj, mrc.Mat, entityID);
		}
		else
		{
			// TODO: 多纹理的支持
			if (mrc.Mat->GetTextures().size() > 0)
				DrawBatch(transform, mfc.GType, mfc.MeshObj, 
					mrc.Mat->GetTextures()[0],
					attribInfo.at("Tiling Factor").second.x,
					attribInfo.at("Color").second,
					entityID);
			else
				DrawBatch(transform, mfc.GType, mfc.MeshObj, attribInfo.at("Color").second, entityID);
		}
	}

	void Renderer3D::ResetStats()
	{
		s_Data.Stats.DrawCalls = 0;
		s_Data.Stats.GeometryCount = 0;
		s_Data.Stats.VertexCount = 0;
		s_Data.Stats.IndexCount = 0;
	}

	RenderStats* Renderer3D::GetStats()
	{
		return &s_Data.Stats;
	}
}