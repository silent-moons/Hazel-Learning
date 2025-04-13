#pragma once

#include "Hazel/Renderer/Texture.h"
#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Renderer/IRenderStats.h"
#include "Hazel/Scene/Components.h"

namespace Hazel
{
	class RenderStats3D : public IRenderStats
	{
	public:
		RenderStats3D() = default;
		uint32_t GetTotalVertexCount() const override { return GeometryCount * 24; }
		uint32_t GetTotalIndexCount() const override { return GeometryCount * 36; }
	};

	class Renderer;
	class Renderer3D
	{
	private:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();
		static void Flush();
		static void Bind();

		static void DrawCube(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
		static void DrawCube(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f), int entityID = -1);
		static void DrawMesh(const glm::mat4& transform, SpriteRendererComponent& src, int entityID);

		static void ResetStats();
		static IRenderStats* GetStats();

		static void StartBatch();
		static void NextBatch();

		friend Renderer;
	};
}