#include "hzpch.h"
#include "Renderer3D.h"

#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/RenderCommand.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Hazel
{
	struct CubeVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;

		// Editor-only
		int EntityID;
	};

	struct Renderer3DData
	{
		static const uint32_t MaxCubes = 1000;
		static const uint32_t MaxVertices = MaxCubes * 24;
		static const uint32_t MaxIndices = MaxCubes * 36;
		static const uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> CubeVA;
		Ref<VertexBuffer> CubeVB;
		Ref<IndexBuffer> CubeIB;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;

		uint32_t CubeIndexCount = 0;
		CubeVertex* CubeVertexBufferBase = nullptr; // 顶点指针起始
		CubeVertex* CubeVertexBufferPtr = nullptr; // 用于移动的顶点指针

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;

		glm::vec4 CubeVertexPositions[24]
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

		RenderStats3D Stats;
	};
	static Renderer3DData s_Data;

	void Renderer3D::Init()
	{
		// 申请画一批三角形需要的最大存储空间，返回其指针
		s_Data.CubeVB = VertexBuffer::Create(s_Data.MaxVertices * sizeof(CubeVertex));

		BufferLayout squareLayout =
		{
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float4, "a_Color"},
			{ShaderDataType::Float2, "a_TexCoord"},
			{ShaderDataType::Float, "a_TexIndex"},
			{ShaderDataType::Float, "a_TilingFactor"},
			{ShaderDataType::Int, "a_EntityID"}
		};

		s_Data.CubeVB->SetLayout(squareLayout);

		// 制作EBO数组
		uint32_t* cubeIndices = new uint32_t[s_Data.MaxIndices];
		uint32_t indices[] = {
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
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 36)
		{
			memcpy(cubeIndices + i, indices, sizeof(indices));
			offset += 24;
			for (int j = 0; j < 36; j++)
				indices[j] += offset;
		}
		s_Data.CubeIB = IndexBuffer::Create(cubeIndices, s_Data.MaxIndices);
		delete[] cubeIndices;

		s_Data.CubeVA = VertexArray::Create();
		s_Data.CubeVA->SetIndexBuffer(s_Data.CubeIB);
		s_Data.CubeVA->AddVertexBuffer(s_Data.CubeVB);
		s_Data.CubeVA->Unbind();

		// 顶点结构体空间
		s_Data.CubeVertexBufferBase = new CubeVertex[s_Data.MaxVertices]; //保存指针初始位置

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
		s_Data.TextureSlotIndex = 1; //每结束（刷新）一次批渲染，需要绘制的纹理索引要从一重新开始，0号固定位白色纹理
		s_Data.CubeVertexBufferPtr = s_Data.CubeVertexBufferBase; // 顶点数据指针置于起点
	}

	void Renderer3D::Flush()
	{
		if (s_Data.CubeIndexCount == 0)
			return;

		// Size 等于后端指针减去前端(hind 在绘制时一直更新数据）
		uint32_t dataSize = uint32_t((uint8_t*)s_Data.CubeVertexBufferPtr - (uint8_t*)s_Data.CubeVertexBufferBase);
		// 将数据送往GPU
		s_Data.CubeVB->SetData(s_Data.CubeVertexBufferBase, dataSize);

		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			s_Data.TextureSlots[i]->Bind(i);

		RenderCommand::DrawIndexed(s_Data.CubeVA, s_Data.CubeIndexCount);
		s_Data.Stats.DrawCalls++;
	}

	void Renderer3D::Bind()
	{
		s_Data.CubeVA->Bind();
	}

	void Renderer3D::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Renderer3D::DrawCube(const glm::mat4& transform, const glm::vec4& color, int entityID)
	{
		constexpr size_t cubeVertexCount = 24;
		const float textureIndex = 0.0f; // White Texture
		constexpr glm::vec2 textureCoords[] =
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
		const float tilingFactor = 1.0f;

		if (s_Data.CubeIndexCount >= Renderer3DData::MaxIndices)
			NextBatch();

		for (size_t i = 0; i < cubeVertexCount; i++)
		{
			s_Data.CubeVertexBufferPtr->Position = transform * s_Data.CubeVertexPositions[i];
			s_Data.CubeVertexBufferPtr->Color = color;
			s_Data.CubeVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.CubeVertexBufferPtr->TexIndex = textureIndex;
			s_Data.CubeVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.CubeVertexBufferPtr->EntityID = entityID;
			s_Data.CubeVertexBufferPtr++;
		}

		s_Data.CubeIndexCount += 36;
		s_Data.Stats.GeometryCount++;
	}

	void Renderer3D::DrawCube(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor, int entityID)
	{
		constexpr size_t cubeVertexCount = 24;
		constexpr glm::vec2 textureCoords[] = 
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

		if (s_Data.CubeIndexCount >= Renderer3DData::MaxIndices)
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

		for (size_t i = 0; i < cubeVertexCount; i++)
		{
			s_Data.CubeVertexBufferPtr->Position = transform * s_Data.CubeVertexPositions[i];
			s_Data.CubeVertexBufferPtr->Color = tintColor;
			s_Data.CubeVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.CubeVertexBufferPtr->TexIndex = textureIndex;
			s_Data.CubeVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.CubeVertexBufferPtr->EntityID = entityID;
			s_Data.CubeVertexBufferPtr++;
		}

		s_Data.CubeIndexCount += 36;
		s_Data.Stats.GeometryCount++;
	}

	void Renderer3D::DrawMesh(const glm::mat4& transform, SpriteRendererComponent& src, int entityID)
	{
		if (src.Texture)
			DrawCube(transform, src.Texture, src.TilingFactor, src.Color, entityID);
		else
			DrawCube(transform, src.Color, entityID);
	}

	void Renderer3D::ResetStats()
	{
		s_Data.Stats.DrawCalls = 0;
		s_Data.Stats.GeometryCount = 0;
	}

	IRenderStats* Renderer3D::GetStats()
	{
		return &s_Data.Stats;
	}
}