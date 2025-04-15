#pragma once

#include "Hazel/Renderer/Texture.h"
#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Scene/Components.h"

namespace Hazel
{
	class Renderer;
	class RenderStats;
	class Renderer3D
	{
	private:
		//static void Init();
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();
		static void Flush();
		static void DrawBatch(
			const glm::mat4& transform, 
			MeshFilterComponent::GeometryType type,
			const Ref<Mesh>& mesh,
			const glm::vec4& color, 
			int entityID = -1);
		static void DrawBatch(
			const glm::mat4& transform,
			MeshFilterComponent::GeometryType type,
			const Ref<Mesh>& mesh,
			const Ref<Texture2D>& texture, 
			float tilingFactor = 1.0f, 
			const glm::vec4& tintColor = glm::vec4(1.0f),
			int entityID = -1);
		static void DrawMesh(const glm::mat4& transform, MeshFilterComponent& mfc, MeshRendererComponent& mrc, int entityID);

		static void ResetStats();
		static RenderStats* GetStats();

		static void StartBatch();
		static void NextBatch();

		friend Renderer;
	};
}