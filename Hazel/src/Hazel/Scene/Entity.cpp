#include "hzpch.h"

#include "Entity.h"
#include "Components.h"
#include "Hazel/Math/Math.h"

namespace Hazel 
{
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene) {}

	Entity Entity::GetParent() const
	{
		UUID parentUUID = GetComponent<TransformComponent>().Parent;
		if (parentUUID == 0)
			return { entt::null, nullptr };
		return { m_Scene->m_EntityMap.at(parentUUID), m_Scene };
	}

	void Entity::SetParent(Entity parent)
	{
		auto& selfTransform = GetComponent<TransformComponent>();
		if (!parent)
		{
			UnBindParent();
			TransformComponent temp;
			ProcessTransform(selfTransform, temp);
			return;
		}
		auto& parentTransform = parent.GetComponent<TransformComponent>();
		UnBindParent();

		// 建立新关系
		selfTransform.Parent = parent.GetUUID();
		ProcessTransform(selfTransform, parentTransform);
		parentTransform.Children.push_back(GetUUID());
	}

	void Entity::UnBindParent()
	{
		auto& tf = GetComponent<TransformComponent>();
		// 清除父子关系（如有）
		if (tf.Parent != 0)
		{
			auto& parentTransform = m_Scene->m_Registry.get<TransformComponent>(m_Scene->m_EntityMap.at(tf.Parent));
			auto it = std::find(parentTransform.Children.begin(), parentTransform.Children.end(), GetUUID());
			parentTransform.Children.erase(it);
		}
		tf.Parent = 0;
	}

	void Entity::ProcessTransform(TransformComponent& child, TransformComponent& parent)
	{
		glm::mat4 newParentWorldTrans = parent.WorldTransform;
		glm::mat4 newLocalTras = glm::inverse(newParentWorldTrans) * child.WorldTransform;
		glm::vec3 translation, rotation, scale;
		Math::DecomposeTransform(newLocalTras, translation, rotation, scale);
		child.Translation = translation;
		child.Rotation = rotation;
		child.Scale = scale;
	}
}