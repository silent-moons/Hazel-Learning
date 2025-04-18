#include "hzpch.h"

#include "Scene.h"
#include "Entity.h"
#include "ScriptableEntity.h"
#include "Hazel/Scripting/ScriptEngine.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Renderer/Renderer3D.h"
#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Renderer/Mesh.h"

// Box2D
#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"

#include <glm/glm.hpp>

namespace Hazel 
{
	static b2BodyType Rigidbody2DTypeToBox2DBody(Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
		case Rigidbody2DComponent::BodyType::Static:    return b2_staticBody;
		case Rigidbody2DComponent::BodyType::Dynamic:   return b2_dynamicBody;
		case Rigidbody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
		}
		HZ_CORE_ASSERT(false, "Unknown body type");
		return b2_staticBody;
	}

	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
		delete m_PhysicsWorld;
	}

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		([&]()
			{
				auto view = src.view<Component>();
				for (auto srcEntity : view)
				{
					entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

					auto& srcComponent = src.get<Component>(srcEntity);
					dst.emplace_or_replace<Component>(dstEntity, srcComponent);
				}
			}(), ...); // (, ...)�۵����ʽ���
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	template<typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		([&]()
			{
				if (src.HasComponent<Component>())
					dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}

	Ref<Scene> Scene::Copy(Ref<Scene> other)
	{
		// �����³���
		Ref<Scene> newScene = CreateRef<Scene>();
		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;
		auto& srcSceneRegistry = other->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;
		std::unordered_map<UUID, entt::entity> enttMap;
		// Create entities in new scene
		auto idView = srcSceneRegistry.view<IDComponent>();
		for (auto e : idView)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
			// Ϊ�³��������;ɳ���ͬ����uuid��ʵ��
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}
		// Copy components (except IDComponent and TagComponent)
		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);
		return newScene;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		m_EntityMap[uuid] = entity;

		return entity;
	}

	void Scene::DuplicateEntity(Entity src, Entity parent)
	{
		// ������ʵ��ͬ������ʵ�壬�������UUID
		Entity newEntity = CreateEntity(src.GetName());
		CopyComponentIfExists(AllComponents{}, newEntity, src);
		auto& newTf = newEntity.GetComponent<TransformComponent>();
		// ��ʵ�����������գ�֮��ݹ齨������ʵ��ĸ�����Ҫ��Ϊ��ǰ�ĸ�����
		newTf.Children.clear();
		
		// �����µĸ��ӹ�ϵ
		if (parent)
		{
			newTf.Parent = parent.GetUUID();
			auto& parentTf = parent.GetComponent<TransformComponent>();
			parentTf.Children.push_back(newEntity.GetUUID());
		}
		auto& srcTf = src.GetComponent<TransformComponent>();
		for (auto child : srcTf.Children)
			DuplicateEntity({ m_EntityMap.at(child), this }, newEntity);
	}

	void Scene::DestroyEntity(Entity entity)
	{
		auto& tf = entity.GetComponent<TransformComponent>();
		for (auto child : tf.Children)
			DestroyEntity({ m_EntityMap.at(child), this });

		// �����ɾ���������и����壬��Ҫ�Ӹ������������������ɾ��
		entity.UnBindParent();
		m_EntityMap.erase(entity.GetUUID());
		m_Registry.destroy(entity);
	}

	void Scene::OnRuntimeStart()
	{
		OnPhysics2DStart();

		// Scripting
		{
			ScriptEngine::OnRuntimeStart(this);
			// Instantiate all script entities
			auto view = m_Registry.view<ScriptComponent>(); // �ҵ�����ӵ�нű������ʵ��
			for (auto e : view)
			{
				Entity entity = { e, this };
				// ʵ����ʵ��ӵ�е�C#�ű��������ýű��е�OnCreate����
				ScriptEngine::OnCreateEntity(entity);
			}
		}
	}

	void Scene::OnRuntimeStop()
	{
		OnPhysics2DStop();
		ScriptEngine::OnRuntimeStop();
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		// Update scripts
		{
			// ����C#�ű�
			auto view = m_Registry.view<ScriptComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				ScriptEngine::OnUpdateEntity(entity, ts);
			}

			// each �����������ͼ�е�����ʵ������ǵ����������ÿ��ʵ�弰�����ִ�лص�����
			// �ص�������ָ������������ʵ�� ID �Ͷ�Ӧ��������á�
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
				{
					if (!nsc.Instance)
					{
						nsc.Instance = nsc.InstantiateScript();
						nsc.Instance->m_Entity = Entity{ entity, this };
						nsc.Instance->OnCreate();
					}
					nsc.Instance->OnUpdate(ts);
				});
		}

		// �������
		{
			const int32_t velocityIterations = 6;
			const int32_t positionIterations = 2;
			// ģ��һ����TODO: �̶�ʱ������ģ�⣬������ʹ��֡��
			m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

			// ��������ģ��֮���״̬
			auto view = m_Registry.view<Rigidbody2DComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

				b2Body* body = (b2Body*)rb2d.RuntimeBody; // �õ���������
				const auto& position = body->GetPosition();
				transform.Translation.x = position.x;
				transform.Translation.y = position.y;
				transform.Rotation.z = body->GetAngle();
			}
		}

		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			// ������ʵ�����Ѽ�����Transform��Camera�����ʵ�壬����ʵ�弯��
			auto view = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = transform.WorldTransform;
					break;
				}
			}
		}

		if (mainCamera)
		{
			Renderer::BeginScene(*mainCamera, cameraTransform);
			switch (Renderer::s_RendererMode)
			{
			case Renderer::Mode::Renderer2D:
			{
				auto view = m_Registry.view<TransformComponent>();
				for (auto entity : view)
				{
					auto& transform = view.get<TransformComponent>(entity);
					if (transform.Parent != 0)
						continue;
					ProcessTree2D(transform, entity);
				}
				break;
			}
			case Renderer::Mode::Renderer3D:
			{
				for (auto& [_, rrifs] : m_BatchGroups)
					rrifs.clear();
				//auto group = m_Registry.group<TransformComponent>(entt::get<MeshRendererComponent>);
				auto view = m_Registry.view<TransformComponent>();
				for (auto entity : view)
				{
					auto& transform = view.get<TransformComponent>(entity);
					if (transform.Parent != 0)
						continue;
					ProcessTree3D(transform, entity);
				}
				for (auto& [_, rrifs] : m_BatchGroups) // ���ƿɺ�������
				{
					for (auto& rrif : rrifs)
					{
						Renderer::Draw(rrif.transform, rrif.mfc, rrif.mrc, rrif.entity);
					}
				}
				break;
			}
			default:
				break;
			}
			Renderer::EndScene();
		}
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		Renderer::BeginScene(camera);
		switch (Renderer::s_RendererMode)
		{
		case Renderer::Mode::Renderer2D:
		{
			auto view = m_Registry.view<TransformComponent>();
			for (auto entity : view)
			{
				auto& transform = view.get<TransformComponent>(entity);
				if (transform.Parent != 0)
					continue;
				ProcessTree2D(transform, entity);
			}
			break;
		}
		case Renderer::Mode::Renderer3D:
		{
			for (auto& [_, rrifs] : m_BatchGroups)
				rrifs.clear();
			
			//auto group = m_Registry.group<TransformComponent>(entt::get<MeshRendererComponent>);
			auto view = m_Registry.view<TransformComponent>();
			for (auto entity : view)
			{
				auto& transform = view.get<TransformComponent>(entity);
				if (transform.Parent != 0)
					continue;
				ProcessTree3D(transform, entity);
			}
			for (auto& [_, rrifs] : m_BatchGroups) // ���ƿɺ�������
			{
				for (auto& rrif : rrifs)
				{
					Renderer::Draw(rrif.transform, rrif.mfc, rrif.mrc, rrif.entity);
				}
			}
			break;
		}
		default:
			break;
		}
		Renderer::EndScene();
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize our non-FixedAspectRatio cameras
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(width, height);
		}
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{ entity, this };
		}
		return {};
	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		// TODO(Yan): Maybe should be assert
		if (m_EntityMap.find(uuid) != m_EntityMap.end())
			return { m_EntityMap.at(uuid), this };
		return {};
	}

	void Scene::OnPhysics2DStart()
	{
		m_PhysicsWorld = new b2World({ 0.0f, -9.8f }); // ����һ���������磬��������9.8

		// ����������ӵ�и��������ʵ��
		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			// ���ø���ĸ�������
			b2BodyDef bodyDef;
			bodyDef.type = Rigidbody2DTypeToBox2DBody(rb2d.Type);
			bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
			bodyDef.angle = transform.Rotation.z;

			// �������õ��������������紴�����壬�����õ���������ڲ�
			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rb2d.FixedRotation);
			rb2d.RuntimeBody = body;

			// �������ײ���������������ײ�����
			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

				// 2D��ײ��
				b2PolygonShape boxShape;
				boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2d.Density;
				fixtureDef.friction = bc2d.Friction;
				fixtureDef.restitution = bc2d.Restitution;
				fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}
		}
	}

	void Scene::OnPhysics2DStop()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		//static_assert(false);
	}
	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}

	// �������в��������ʱ�������CameraComponent����Ҫ��һʱ�������ӿڣ��������ӿڸı�֮ǰ���޷���Ⱦ
	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}
	template<>
	void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<MeshRendererComponent>(Entity entity, MeshRendererComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
	{
	}

	void Scene::ProcessTree2D(TransformComponent& transform, entt::entity& entity)
	{
		// ���ڵ������任���Ǳ��ر任���Ǹ��ڵ������任Ϊ���ڵ�����任�����Լ��ı��ر任
		if (transform.Parent != 0)
		{
			auto& parentTransform = m_Registry.get<TransformComponent>(m_EntityMap.at(transform.Parent));
			transform.WorldTransform = parentTransform.WorldTransform * transform.LocalTransform();
		}
		else
			transform.WorldTransform = transform.LocalTransform();

		// ���ʵ����SpriteRenderer���һ������
		if (m_Registry.all_of<SpriteRendererComponent>(entity))
		{
			auto src = m_Registry.get<SpriteRendererComponent>(entity);
			Renderer::Draw(transform, src, entity);
		}
		for (auto& child : transform.Children)
		{
			auto& childTransform = m_Registry.get<TransformComponent>(m_EntityMap.at(child));
			ProcessTree2D(childTransform, m_EntityMap.at(child));
		}
	}

	void Scene::ProcessTree3D(TransformComponent& transform, entt::entity& entity)
	{
		// ���ڵ������任���Ǳ��ر任���Ǹ��ڵ������任Ϊ���ڵ�����任�����Լ��ı��ر任
		if (transform.Parent != 0)
		{
			auto& parentTransform = m_Registry.get<TransformComponent>(m_EntityMap.at(transform.Parent));
			transform.WorldTransform = parentTransform.WorldTransform * transform.LocalTransform();
		}
		else
			transform.WorldTransform = transform.LocalTransform();

		// ���ʵ����MeshFilter�����MeshRenderer���һ������
		if (m_Registry.all_of<MeshFilterComponent, MeshRendererComponent>(entity))
		{
			auto [mfc, mrc] = m_Registry.get<MeshFilterComponent, MeshRendererComponent>(entity);
			Mesh* meshptr = mfc.MeshObj.get();
			switch (mfc.MeshObj->GetMeshType())
			{
			case MeshType::StaticBatchable:
			{
				auto meshptr = mfc.MeshObj.get();
				m_BatchGroups[meshptr].emplace_back(transform, mfc, mrc, entity);
				break;
			}
			case MeshType::StaticUnique: // δ��������ֱ�ӵ�����Ⱦ����
				break;
			case MeshType::SkinnedMesh:
				break;
			default:
				break;
			}
		}
		for (auto& child : transform.Children)
		{
			auto& childTransform = m_Registry.get<TransformComponent>(m_EntityMap.at(child));
			ProcessTree3D(childTransform, m_EntityMap.at(child));
		}
	}
}