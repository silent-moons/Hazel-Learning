#pragma once

#include "Hazel/Renderer/Texture.h"
#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Scene/Components.h"

namespace Hazel
{
	class Renderer3D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();
		static void Flush();

		static void DrawCube(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
		static void DrawCube(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f), int entityID = -1);
		static void DrawCube(const glm::mat4& transform, SpriteRendererComponent& src, int entityID);

		// Statistics (����ʱʹ�õ�ͳ�����ݣ������Statistics�ṹ��)
		struct Statistics
		{
			uint32_t DrawCalls;
			uint32_t CubeCount;
			// ����������ʱ�ټ���Vertex��Index����ʡ����
			uint32_t GetTotalVertexCount() const { return CubeCount * 24; }
			uint32_t GetTotalIndexCount() const { return CubeCount * 36; }
		};
		static void ResetStats();
		static Statistics GetStats();

	private:
		static void StartBatch();
		static void NextBatch();
	};
}