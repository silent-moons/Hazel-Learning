#pragma once

#include <entt.hpp>

#include "Hazel/Core/Timestep.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Scene/Components.h"

class b2World;

namespace Hazel 
{
	class Entity;
	class Mesh;
	class Scene
	{
	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> other);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnUpdateRuntime(Timestep ts);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);

		void DuplicateEntity(Entity entity);

		Entity GetEntityByUUID(UUID uuid);
		Entity GetPrimaryCameraEntity();

	private:
		void OnPhysics2DStart();
		void OnPhysics2DStop();

		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		void ProcessTree(TransformComponent& transform, entt::entity& entity);
	private:
		struct RenderRequiredInfos
		{
			TransformComponent transform;
			MeshFilterComponent mfc;
			MeshRendererComponent mrc;
			entt::entity entity;

			RenderRequiredInfos() = default;
			RenderRequiredInfos(
				TransformComponent& transform,
				MeshFilterComponent& mfc,
				MeshRendererComponent& mrc,
				entt::entity& entity
			) : transform(transform), mfc(mfc), mrc(mrc), entity(entity) {}
		};

		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		b2World* m_PhysicsWorld = nullptr;
		std::unordered_map<UUID, entt::entity> m_EntityMap;
		std::unordered_map<Mesh*, std::vector<RenderRequiredInfos>> m_BatchGroups;
		std::string m_Name = "Untitled";

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}