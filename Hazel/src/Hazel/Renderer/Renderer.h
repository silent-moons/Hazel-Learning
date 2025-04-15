#pragma once

#include "RenderCommand.h"
#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Scene/Components.h"
#include "Renderer2D.h"
#include "Renderer3D.h"

namespace Hazel
{
	class RenderStats;
	class Renderer
	{
	public:
		enum class Mode 
		{
			Renderer2D, Renderer3D
		};
	public:
		static void Init();
		static void Shutdown();
		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();
		static void Draw(const glm::mat4& transform, SpriteRendererComponent& src, int entityID);
		static void Draw(const glm::mat4& transform, MeshFilterComponent& mfc, MeshRendererComponent& mrc, int entityID);
		static void ResetStats();
		static RenderStats* GetStats();

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
		static void SetMode(Mode mode);

		static Mode s_RendererMode;
	private:
		static std::function<void(const Camera&, const glm::mat4&)> s_BeginSceneRuntimeFn;
		static std::function<void(const EditorCamera&)> s_BeginSceneEditorFn;
		static std::function<void(const glm::mat4&, SpriteRendererComponent&, int)> s_DrawSpriteFn;
		static std::function<void(const glm::mat4&, MeshFilterComponent&, MeshRendererComponent&, int)> s_DrawMeshFn;
		static std::function<void()> s_EndSceneFn;
		static std::function<void()> s_ResetStatsFn;
		static std::function<RenderStats*()> s_GetStatsFn;
	};
}