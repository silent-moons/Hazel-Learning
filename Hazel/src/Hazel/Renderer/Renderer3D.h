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

		// 可合批物体Runtime和Editor
		static void BeginBatch(const Camera& camera, const glm::mat4& transform);
		static void BeginBatch(const EditorCamera& camera);
		// 不可合批物体Runtime和Editor
		static void BeginUnique(const Camera& camera, const glm::mat4& modelMat, const glm::mat4& cameraTrans);
		static void BeginUnique(const EditorCamera& camera, const glm::mat4& modelMat);
		static void EndBatch();
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
		static void DrawUnique(
			const glm::mat4& transform,
			const Ref<Mesh>& mesh,
			const Ref<Texture2D>& texture,
			int entityID = -1);
		static void DrawMesh(const glm::mat4& transform, MeshFilterComponent& mfc, MeshRendererComponent& mrc, int entityID);

		static void ResetStats();
		static RenderStats* GetStats();

		static void StartBatch();
		static void NextBatch();

		friend Renderer;
	};
}