#include "hzpch.h"
#include "Renderer3D.h"

#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/RenderStats.h"
#include "Hazel/Renderer/Geometry/Cube.h"
#include "Hazel/Renderer/Geometry/Sphere.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Hazel
{
	struct Renderer3DData
	{
		static constexpr uint32_t MaxCubes = 100;
		static constexpr uint32_t MaxCubeVertices = MaxCubes * Cube::GetVertexCount();
		static constexpr uint32_t MaxCubeIndices = MaxCubes * Cube::GetIndexCount();
		static constexpr uint32_t MaxSpheres = 50;
		static constexpr uint32_t MaxSphereVertices = MaxSpheres * Sphere::GetVertexCount();
		static constexpr uint32_t MaxSphereIndices = MaxSpheres * Sphere::GetIndexCount();
		static constexpr uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> CubeVA;
		Ref<VertexBuffer> CubeVB;
		Ref<IndexBuffer> CubeIB;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;

		Ref<VertexArray> SphereVA;
		Ref<VertexBuffer> SphereVB;
		Ref<IndexBuffer> SphereIB;

		uint32_t CubeIndexCount = 0;
		CubeVertex* CubeVertexBufferBase = nullptr; // 顶点指针起始
		CubeVertex* CubeVertexBufferPtr = nullptr; // 用于移动的顶点指针
		uint32_t SphereIndexCount = 0;
		SphereVertex* SphereVertexBufferBase = nullptr;
		SphereVertex* SphereVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;

		RenderStats Stats;
	};
	static Renderer3DData s_Data;

	void Renderer3D::Init()
	{
		// 立方体合批
		s_Data.CubeVB = VertexBuffer::Create(s_Data.MaxCubeVertices * sizeof(CubeVertex));
		BufferLayout cubeLayout =
		{
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float4, "a_Color"},
			{ShaderDataType::Float2, "a_TexCoord"},
			{ShaderDataType::Float, "a_TexIndex"},
			{ShaderDataType::Float, "a_TilingFactor"},
			{ShaderDataType::Int, "a_EntityID"}
		};

		s_Data.CubeVB->SetLayout(cubeLayout);

		// 制作EBO数组
		uint32_t* cubeIndices = new uint32_t[s_Data.MaxCubeIndices];
		std::vector<uint32_t> indices = Cube::GetIndices();
		for (uint32_t i = 0; i < s_Data.MaxCubeIndices; i += Cube::GetIndexCount())
		{
			memcpy(cubeIndices + i, indices.data(), indices.size() * sizeof(uint32_t));
			for (int j = 0; j < Cube::GetIndexCount(); j++)
				indices[j] += Cube::GetVertexCount();
		}
		s_Data.CubeIB = IndexBuffer::Create(cubeIndices, s_Data.MaxCubeIndices);
		delete[] cubeIndices;

		s_Data.CubeVA = VertexArray::Create();
		s_Data.CubeVA->SetIndexBuffer(s_Data.CubeIB);
		s_Data.CubeVA->AddVertexBuffer(s_Data.CubeVB);
		s_Data.CubeVA->Unbind();
		s_Data.CubeVertexBufferBase = new CubeVertex[s_Data.MaxCubeVertices]; //保存指针初始位置

		// 球体合批
		s_Data.SphereVB = VertexBuffer::Create(s_Data.MaxSphereVertices * sizeof(SphereVertex));
		s_Data.SphereVB->SetLayout(cubeLayout);

		// 制作EBO数组
		uint32_t* sphereIndices = new uint32_t[s_Data.MaxSphereIndices];
		indices = Sphere::GetIndices();
		for (uint32_t i = 0; i < s_Data.MaxCubeIndices; i += Sphere::GetIndexCount())
		{
			memcpy(sphereIndices + i, indices.data(), indices.size() * sizeof(uint32_t));
			for (int j = 0; j < Sphere::GetIndexCount(); j++)
				indices[j] += Sphere::GetVertexCount();
		}
		s_Data.SphereIB = IndexBuffer::Create(sphereIndices, s_Data.MaxSphereIndices);
		delete[] sphereIndices;

		s_Data.SphereVA = VertexArray::Create();
		s_Data.SphereVA->SetIndexBuffer(s_Data.SphereIB);
		s_Data.SphereVA->AddVertexBuffer(s_Data.SphereVB);
		s_Data.SphereVA->Unbind();
		s_Data.SphereVertexBufferBase = new SphereVertex[s_Data.MaxSphereVertices]; //保存指针初始位置

		// 纹理采样器
		int32_t samplers[s_Data.MaxTextureSlots];
		for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
			samplers[i] = i;

		// Shader
		s_Data.TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		s_Data.TextureShader->Bind();
		//上传所有采样器到对应纹理单元
		s_Data.TextureShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

		// Texture
		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
		s_Data.TextureSlots[0] = s_Data.WhiteTexture;
	}

	void Renderer3D::Shutdown()
	{
		delete[] s_Data.CubeVertexBufferBase;
		delete[] s_Data.SphereVertexBufferBase;
	}

	void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);
		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_ViewProjection", viewProj);
		StartBatch();
	}

	void Renderer3D::BeginScene(const EditorCamera& camera)
	{
		glm::mat4 viewProj = camera.GetViewProjection();
		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_ViewProjection", viewProj);
		StartBatch();
	}

	void Renderer3D::EndScene()
	{
		Flush();
	}

	void Renderer3D::StartBatch()
	{
		s_Data.CubeIndexCount = 0; //每结束（刷新）一次批渲染，需要绘制的索引数要从零重新开始
		s_Data.CubeVertexBufferPtr = s_Data.CubeVertexBufferBase; // 顶点数据指针置于起点
		s_Data.SphereIndexCount = 0;
		s_Data.SphereVertexBufferPtr = s_Data.SphereVertexBufferBase;

		s_Data.TextureSlotIndex = 1; //每结束（刷新）一次批渲染，需要绘制的纹理索引要从一重新开始，0号固定位白色纹理
	}

	void Renderer3D::Flush()
	{
		if (s_Data.CubeIndexCount)
		{
			// Size 等于后端指针减去前端
			uint32_t dataSize = uint32_t((uint8_t*)s_Data.CubeVertexBufferPtr - (uint8_t*)s_Data.CubeVertexBufferBase);
			// 将数据送往GPU
			s_Data.CubeVB->SetData(s_Data.CubeVertexBufferBase, dataSize);

			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
				s_Data.TextureSlots[i]->Bind(i);

			RenderCommand::DrawIndexed(s_Data.CubeVA, s_Data.CubeIndexCount);
			s_Data.Stats.DrawCalls++;
		}
		if (s_Data.SphereIndexCount)
		{
			// Size 等于后端指针减去前端
			uint32_t dataSize = uint32_t((uint8_t*)s_Data.SphereVertexBufferPtr - (uint8_t*)s_Data.SphereVertexBufferBase);
			// 将数据送往GPU
			s_Data.SphereVB->SetData(s_Data.SphereVertexBufferBase, dataSize);

			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
				s_Data.TextureSlots[i]->Bind(i);

			RenderCommand::DrawIndexed(s_Data.SphereVA, s_Data.SphereIndexCount);
			s_Data.Stats.DrawCalls++;
		}
	}

	void Renderer3D::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Renderer3D::DrawCube(const glm::mat4& transform, const glm::vec4& color, int entityID)
	{
		const float textureIndex = 0.0f; // White Texture
		const float tilingFactor = 1.0f;

		if (s_Data.CubeIndexCount >= Renderer3DData::MaxCubeIndices)
			NextBatch();

		for (size_t i = 0; i < Cube::GetVertexCount(); i++)
		{
			s_Data.CubeVertexBufferPtr->Position = transform * Cube::GetVertices()[i];
			s_Data.CubeVertexBufferPtr->Color = color;
			s_Data.CubeVertexBufferPtr->TexCoord = Cube::GetTextureCoords()[i];
			s_Data.CubeVertexBufferPtr->TexIndex = textureIndex;
			s_Data.CubeVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.CubeVertexBufferPtr->EntityID = entityID;
			s_Data.CubeVertexBufferPtr++;
		}

		s_Data.CubeIndexCount += Cube::GetIndexCount();
		s_Data.Stats.VertexCount += Cube::GetVertexCount();
		s_Data.Stats.IndexCount += Cube::GetIndexCount();
		s_Data.Stats.GeometryCount++;
	}

	void Renderer3D::DrawCube(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor, int entityID)
	{
		if (s_Data.CubeIndexCount >= Renderer3DData::MaxCubeIndices)
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

		for (size_t i = 0; i < Cube::GetVertexCount(); i++)
		{
			s_Data.CubeVertexBufferPtr->Position = transform * Cube::GetVertices()[i];
			s_Data.CubeVertexBufferPtr->Color = tintColor;
			s_Data.CubeVertexBufferPtr->TexCoord = Cube::GetTextureCoords()[i];
			s_Data.CubeVertexBufferPtr->TexIndex = textureIndex;
			s_Data.CubeVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.CubeVertexBufferPtr->EntityID = entityID;
			s_Data.CubeVertexBufferPtr++;
		}

		s_Data.CubeIndexCount += Cube::GetIndexCount();
		s_Data.Stats.VertexCount += Cube::GetVertexCount();
		s_Data.Stats.IndexCount += Cube::GetIndexCount();
		s_Data.Stats.GeometryCount++;
	}

	void Renderer3D::DrawSphere(const glm::mat4& transform, const glm::vec4& color, int entityID)
	{
		const float textureIndex = 0.0f; // White Texture
		const float tilingFactor = 1.0f;

		if (s_Data.SphereIndexCount >= Renderer3DData::MaxSphereIndices)
			NextBatch();

		for (size_t i = 0; i < Sphere::GetVertexCount(); i++)
		{
			s_Data.SphereVertexBufferPtr->Position = transform * Sphere::GetVertices()[i];
			s_Data.SphereVertexBufferPtr->Color = color;
			s_Data.SphereVertexBufferPtr->TexCoord = Sphere::GetTextureCoords()[i];
			s_Data.SphereVertexBufferPtr->TexIndex = textureIndex;
			s_Data.SphereVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.SphereVertexBufferPtr->EntityID = entityID;
			s_Data.SphereVertexBufferPtr++;
		}

		s_Data.SphereIndexCount += Sphere::GetIndexCount();
		s_Data.Stats.VertexCount += Sphere::GetVertexCount();
		s_Data.Stats.IndexCount += Sphere::GetIndexCount();
		s_Data.Stats.GeometryCount++;
	}

	void Renderer3D::DrawSphere(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor, int entityID)
	{
		if (s_Data.SphereIndexCount >= Renderer3DData::MaxSphereIndices)
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

		for (size_t i = 0; i < Sphere::GetVertexCount(); i++)
		{
			s_Data.SphereVertexBufferPtr->Position = transform * Sphere::GetVertices()[i];
			s_Data.SphereVertexBufferPtr->Color = tintColor;
			s_Data.SphereVertexBufferPtr->TexCoord = Sphere::GetTextureCoords()[i];
			s_Data.SphereVertexBufferPtr->TexIndex = textureIndex;
			s_Data.SphereVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.SphereVertexBufferPtr->EntityID = entityID;
			s_Data.SphereVertexBufferPtr++;
		}

		s_Data.SphereIndexCount += Sphere::GetIndexCount();
		s_Data.Stats.VertexCount += Sphere::GetVertexCount();
		s_Data.Stats.IndexCount += Sphere::GetIndexCount();
		s_Data.Stats.GeometryCount++;
	}

	void Renderer3D::DrawMesh(const glm::mat4& transform, MeshFilterComponent& mesh, MeshRendererComponent& mrc, int entityID)
	{
		if (mrc.Texture)
		{
			if (mesh.Name == "Cube")
				DrawCube(transform, mrc.Texture, mrc.TilingFactor, mrc.Color, entityID);
			else if(mesh.Name == "Sphere")
				DrawSphere(transform, mrc.Texture, mrc.TilingFactor, mrc.Color, entityID);
		}
		else
		{
			if (mesh.Name == "Cube")
				DrawCube(transform, mrc.Color, entityID);
			else if (mesh.Name == "Sphere")
				DrawSphere(transform, mrc.Color, entityID);
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