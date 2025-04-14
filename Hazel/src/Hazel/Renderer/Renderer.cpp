#include "hzpch.h"
#include "Renderer.h"

namespace Hazel 
{
	Renderer::Mode Renderer::s_RendererMode = Mode::Renderer3D;
	std::function<void(const Camera&, const glm::mat4&)> Renderer::s_BeginSceneRuntimeFn = nullptr;
	std::function<void(const EditorCamera&)> Renderer::s_BeginSceneEditorFn = nullptr;
	std::function<void(const glm::mat4&, SpriteRendererComponent&, int)> Renderer::s_DrawSpriteFn = nullptr;
	std::function<void(const glm::mat4&, MeshFilterComponent&, MeshRendererComponent&, int)> Renderer::s_DrawMeshFn = nullptr;
	std::function<void()> Renderer::s_EndSceneFn = nullptr;
	std::function<void()> Renderer::s_ResetStatsFn = nullptr;
	std::function<IRenderStats*()> Renderer::s_GetStatsFn = nullptr;

	void Renderer::Init()
	{
		RenderCommand::Init();
		Renderer2D::Init();
		Renderer3D::Init();
		SetMode(Mode::Renderer3D);
	}

	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
		Renderer3D::Shutdown();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		if (s_BeginSceneRuntimeFn)
			s_BeginSceneRuntimeFn(camera, transform);
		else
			HZ_CORE_ERROR("Renderer::BeginScene: No function bound!");
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		if (s_BeginSceneEditorFn)
			s_BeginSceneEditorFn(camera);
		else
			HZ_CORE_ERROR("Renderer::BeginScene: No function bound!");
	}

	void Renderer::EndScene()
	{
		if(s_EndSceneFn)
			s_EndSceneFn();
		else
			HZ_CORE_ERROR("Renderer::EndScene: No function bound!");
	}

	void Renderer::Draw(const glm::mat4& transform, SpriteRendererComponent& src, int entityID)
	{
		if(s_DrawSpriteFn)
			s_DrawSpriteFn(transform, src, entityID);
		else
			HZ_CORE_ERROR("Renderer::Draw: No function bound!");
	}

	void Renderer::Draw(const glm::mat4& transform, MeshFilterComponent& mesh, MeshRendererComponent& mrc, int entityID)
	{
		if (s_DrawMeshFn)
			s_DrawMeshFn(transform, mesh, mrc, entityID);
		else
			HZ_CORE_ERROR("Renderer::Draw: No function bound!");
	}

	void Renderer::ResetStats()
	{
		if (s_ResetStatsFn)
			s_ResetStatsFn();
		else
			HZ_CORE_ERROR("Renderer::ResetStats: No function bound!");
	}

	IRenderStats* Renderer::GetStats()
	{
		if (s_GetStatsFn)
			return s_GetStatsFn();
		HZ_CORE_ERROR("Renderer::GetStats: No function bound!");
		return nullptr;
	}

	void Renderer::SetMode(Mode mode) 
	{
		switch (mode) 
		{
		case Mode::Renderer2D:
			s_RendererMode = Mode::Renderer2D;
			s_BeginSceneRuntimeFn = [](const Camera& camera, const glm::mat4& transform) { Renderer2D::BeginScene(camera, transform); };
			s_BeginSceneEditorFn = [](const EditorCamera& camera) { Renderer2D::BeginScene(camera); };
			s_DrawSpriteFn = Renderer2D::DrawSprite;
			s_EndSceneFn = Renderer2D::EndScene;
			s_ResetStatsFn = Renderer2D::ResetStats;
			s_GetStatsFn = Renderer2D::GetStats;
			break;
		case Mode::Renderer3D:
			s_RendererMode = Mode::Renderer3D;
			s_BeginSceneRuntimeFn = [](const Camera& camera, const glm::mat4& transform) { Renderer3D::BeginScene(camera, transform); };
			s_BeginSceneEditorFn = [](const EditorCamera& camera) { Renderer3D::BeginScene(camera); };
			s_DrawMeshFn = [](const glm::mat4& transform, MeshFilterComponent& mesh, MeshRendererComponent& mrc, int entityID) { Renderer3D::DrawMesh(transform, mesh, mrc, entityID); };
			s_EndSceneFn = Renderer3D::EndScene;
			s_ResetStatsFn = Renderer3D::ResetStats;
			s_GetStatsFn = Renderer3D::GetStats;
			break;
		}
	}

	std::string Renderer::GetModeString(Mode mode)
	{
		return mode == Mode::Renderer2D ? "2D" : "3D";
	}
}