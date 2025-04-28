#include "hzpch.h"

#include "Renderer.h"
#include "RenderStats.h"

namespace Hazel 
{
	Renderer::Mode Renderer::s_RendererMode = Mode::Renderer3D;
	std::function<void(const Camera&, const glm::mat4&)> Renderer::s_BeginBatchRuntimeFn = nullptr;
	std::function<void(const EditorCamera&)> Renderer::s_BeginBatchEditorFn = nullptr;
	std::function<void(const glm::mat4&, SpriteRendererComponent&, int)> Renderer::s_DrawSpriteFn = nullptr;
	std::function<void(const Camera& camera, const glm::mat4&, const glm::mat4&, MeshFilterComponent&, MeshRendererComponent&, int)> Renderer::s_DrawMeshFn = nullptr;
	std::function<void()> Renderer::s_EndBatchFn = nullptr;
	std::function<void()> Renderer::s_ResetStatsFn = nullptr;
	std::function<RenderStats*()> Renderer::s_GetStatsFn = nullptr;

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

	void Renderer::BeginBatch(const Camera& camera, const glm::mat4& cameraTrans)
	{
		if (s_BeginBatchRuntimeFn)
			s_BeginBatchRuntimeFn(camera, cameraTrans);
		else
			HZ_CORE_ERROR("Renderer::BeginBatch: No function bound!");
	}

	void Renderer::BeginBatch(const EditorCamera& camera)
	{
		if (s_BeginBatchEditorFn)
			s_BeginBatchEditorFn(camera);
		else
			HZ_CORE_ERROR("Renderer::BeginBatch: No function bound!");
	}

	void Renderer::EndBatch()
	{
		if(s_EndBatchFn)
			s_EndBatchFn();
		else
			HZ_CORE_ERROR("Renderer::EndBatch: No function bound!");
	}

	void Renderer::Draw(TransformComponent& transform, SpriteRendererComponent& src, entt::entity entityID)
	{
		if(s_DrawSpriteFn)
			s_DrawSpriteFn(transform.WorldTransform, src, (int)entityID);
		else
			HZ_CORE_ERROR("Renderer::Draw: No function bound!");
	}

	void Renderer::Draw(const Camera& camera, const glm::mat4& cameraTrans, TransformComponent& transform, MeshFilterComponent& mfc, MeshRendererComponent& mrc, entt::entity entityID)
	{
		if (s_DrawMeshFn)
			s_DrawMeshFn(camera, cameraTrans, transform.WorldTransform, mfc, mrc, (int)entityID);
		else
			HZ_CORE_ERROR("Renderer::Draw: No function bound!");
	}

	void Renderer::Draw(const Camera& camera, TransformComponent& transform, MeshFilterComponent& mfc, MeshRendererComponent& mrc, entt::entity entityID)
	{
		if (s_DrawMeshFn)
			s_DrawMeshFn(camera, glm::mat4(1.0f), transform.WorldTransform, mfc, mrc, (int)entityID);
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

	RenderStats* Renderer::GetStats()
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
			s_BeginBatchRuntimeFn = [](const Camera& camera, const glm::mat4& transform) { Renderer2D::BeginBatch(camera, transform); };
			s_BeginBatchEditorFn = [](const EditorCamera& camera) { Renderer2D::BeginBatch(camera); };
			s_DrawSpriteFn = Renderer2D::DrawSprite;
			s_EndBatchFn = Renderer2D::EndBatch;
			s_ResetStatsFn = Renderer2D::ResetStats;
			s_GetStatsFn = Renderer2D::GetStats;
			break;
		case Mode::Renderer3D:
			s_RendererMode = Mode::Renderer3D;
			s_BeginBatchRuntimeFn = [](const Camera& camera, const glm::mat4& transform) { Renderer3D::BeginBatch(camera, transform); };
			s_BeginBatchEditorFn = [](const EditorCamera& camera) { Renderer3D::BeginBatch(camera); };
			s_DrawMeshFn = Renderer3D::DrawMesh;
			s_EndBatchFn = Renderer3D::EndBatch;
			s_ResetStatsFn = Renderer3D::ResetStats;
			s_GetStatsFn = Renderer3D::GetStats;
			break;
		}
	}
}