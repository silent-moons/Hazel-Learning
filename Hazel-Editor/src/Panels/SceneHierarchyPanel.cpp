#include "SceneHierarchyPanel.h"

#include <filesystem>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

#include "Hazel/Scene/Components.h"
#include "Hazel/Scripting/ScriptEngine.h"

namespace Hazel 
{

	extern const std::filesystem::path g_AssetPath;

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
		m_SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		if (m_Context)
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Always);
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen
										| ImGuiTreeNodeFlags_FramePadding;

			if (ImGui::TreeNodeEx("##TreeNode", flags))
			{
				char buffer[128];
				strncpy(buffer, m_Context->m_Name.c_str(), sizeof(buffer));
				ImGui::SameLine();
				ImGui::SetNextItemWidth(150);
				if (ImGui::InputText("##NodeNameInput", buffer, 128))
					m_Context->m_Name = buffer;
				
				// 在场景名树节点下绘制实体
				m_Context->m_Registry.view<TransformComponent>().each(
					[&](auto entityID, TransformComponent& tf)
					{
						Entity entity{ entityID , m_Context.get() };
						if (tf.Parent == 0)
							DrawEntityNode(entity);
					});
				ImGui::TreePop();
			}

			// 鼠标点击窗口空白处，将当前选择的实体置为空，下方将不再渲染实体对应的组件
			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				m_SelectionContext = {};

			// 创建一个覆盖整个窗口的 InvisibleButton 作为接受区域
			ImVec2 winSize = ImGui::GetContentRegionAvail();
			ImVec2 winPos = ImGui::GetCursorScreenPos();
			ImGui::SetCursorScreenPos(winPos); // 重置光标位置
			ImGui::InvisibleButton("##DropZone", winSize);
			
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				ImGui::OpenPopup("HierarchyContextMenu");
			if (ImGui::BeginPopup("HierarchyContextMenu"))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
					m_Context->CreateEntity("Empty Entity");
				ImGui::EndPopup();
			}

			// 拖拽实体到空白处
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
				{
					Entity* droppedHandle = (Entity*)payload->Data;
					Entity droppedEntity = { (entt::entity)(*droppedHandle), m_Context.get() };
					droppedEntity.SetParent({ (entt::entity)0, nullptr });
				}
				ImGui::EndDragDropTarget();
			}
		}
		ImGui::End();

		ImGui::Begin("Properties");
		if (m_SelectionContext)
		{
			DrawComponents(m_SelectionContext);
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::SetSelectedEntity(Entity entity)
	{
		m_SelectionContext = entity;
		// 防止在视口点选实体时，组件面板没有失焦
		// 如果没有这句代码，点选实体时，组件面板的某个控件在激活状态，例如tag输入框
		// 那么点选后的实体tag就会变成上一个实体的tag，因为它们有相同的控件ID
		// 所以必须保证每次点选实体时，组件面板要失焦
		// 在层级面板中点选实体时，层级面板会自动聚焦，组件面板自动失焦，所以没有上述问题
		ImGui::SetWindowFocus("Scene Hierarchy");
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		auto& transform = entity.GetComponent<TransformComponent>();
		bool hasChildren = !transform.Children.empty();

		// 节点是否被选中，节点箭头是否是打开状态
		ImGuiTreeNodeFlags flags = (m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0;
		flags |= ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		if (!hasChildren)
			flags |= ImGuiTreeNodeFlags_Leaf;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
			m_SelectionContext = entity;

		// 拖拽开始
		if (ImGui::BeginDragDropSource()) 
		{
			m_DragDropSource = entity;
			ImGui::SetDragDropPayload("ENTITY", &m_DragDropSource, sizeof(Entity));
			ImGui::Text("Set Parent");
			ImGui::EndDragDropSource();
		}

		// 拖拽目标
		if (ImGui::BeginDragDropTarget()) 
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY")) 
			{
				Entity* droppedHandle = (Entity*)payload->Data;
				if (droppedHandle != &entity) 
				{
					Entity droppedEntity = { (entt::entity)(*droppedHandle), m_Context.get()};
					droppedEntity.SetParent(entity);
				}
			}
			ImGui::EndDragDropTarget();
		}

		// 在树形菜单的项目上右击，弹出删除菜单
		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;
			ImGui::EndPopup();
		}

		if (opened)
		{
			// 递归绘制实体的子节点
			for (auto child : transform.Children)
				DrawEntityNode(m_Context->GetEntityByUUID(child));
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
		}
	}

	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		// 分为两列，左边为Translation/rotation/scale，右边为向量值
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth); // 设置第0列的列宽
		ImGui::Text(label.c_str());
		ImGui::NextColumn(); // 切换到下一列

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth()); // 设置3个item，动态调整宽度
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 }); // 项目之间的水平竖直间距都设为0

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize)) // 点击按钮会重置X的值
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine(); // 将下一个item放在同一行
		// 使用##用来分隔控件的显示标签和其标识符。
		// 例如，"Button##UniqueID"中，"Button"是用户在界面上看到的文本，而UniqueID是 ImGui 用来跟踪该控件的内部标识符。
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"); // 可以拖动改变X的值
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		
		// 实体拥有该组件时再画
		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings"); // +号按钮的弹出菜单
			}
			bool removeComponent = false;
			// 目前菜单可以选择删除组件
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;
				ImGui::EndPopup();
			}
			if (open) // 如果组件菜单是打开状态，就回调传入的不同组件的专用绘制函数
			{
				uiFunction(component);
				ImGui::TreePop();
			}
			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			std::strncpy(buffer, tag.c_str(), sizeof(buffer));
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		ImGui::SameLine(); // 添加组件的按钮放在同一行
		ImGui::PushItemWidth(-1);
		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent"); // 弹出菜单
		if (ImGui::BeginPopup("AddComponent")) // 允许弹出窗口
		{
			// TODO: 随着组件的增加，这里会不断修改，考虑更好的设计
			DisplayAddComponentEntry<CameraComponent>("Camera");
			DisplayAddComponentEntry<ScriptComponent>("Script");
			DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
			DisplayAddComponentEntry<MeshFilterComponent>("Mesh Filter");
			DisplayAddComponentEntry<MeshRendererComponent>("Mesh Renderer");
			DisplayAddComponentEntry<Rigidbody2DComponent>("Rigidbody 2D");
			DisplayAddComponentEntry<BoxCollider2DComponent>("Box Collider 2D");
			ImGui::EndPopup();
		}
		ImGui::PopItemWidth();

		// TODO: 与上面类似，每新定义一个组件，都要在这里添加绘制的具体逻辑
		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
			{
				DrawVec3Control("Translation", component.Translation);
				glm::vec3 rotation = glm::degrees(component.Rotation);
				DrawVec3Control("Rotation", rotation);
				component.Rotation = glm::radians(rotation);
				DrawVec3Control("Scale", component.Scale, 1.0f);
			});

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
			{
				auto& camera = component.Camera;

				ImGui::Checkbox("Primary", &component.Primary);

				const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
				const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
				if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
				{
					for (int i = 0; i < 2; i++)
					{
						bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
						if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
						{
							currentProjectionTypeString = projectionTypeStrings[i];
							camera.SetProjectionType((SceneCamera::ProjectionType)i);
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float perspectiveVerticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
					if (ImGui::DragFloat("Vertical FOV", &perspectiveVerticalFov))
						camera.SetPerspectiveVerticalFOV(glm::radians(perspectiveVerticalFov));

					float perspectiveNear = camera.GetPerspectiveNearClip();
					if (ImGui::DragFloat("Near", &perspectiveNear))
						camera.SetPerspectiveNearClip(perspectiveNear);

					float perspectiveFar = camera.GetPerspectiveFarClip();
					if (ImGui::DragFloat("Far", &perspectiveFar))
						camera.SetPerspectiveFarClip(perspectiveFar);
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float orthoSize = camera.GetOrthographicSize();
					if (ImGui::DragFloat("Size", &orthoSize))
						camera.SetOrthographicSize(orthoSize);

					float orthoNear = camera.GetOrthographicNearClip();
					if (ImGui::DragFloat("Near", &orthoNear))
						camera.SetOrthographicNearClip(orthoNear);

					float orthoFar = camera.GetOrthographicFarClip();
					if (ImGui::DragFloat("Far", &orthoFar))
						camera.SetOrthographicFarClip(orthoFar);

					ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
				}
			});

		// lambda捕获的变量默认为const，所以给lambda声明为mutable，把捕获的变量去除const
		DrawComponent<ScriptComponent>("Script", entity, [entity](auto& component) mutable
			{
				bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);
				static char buffer[64];
				strcpy(buffer, component.ClassName.c_str());
				// 如果输入框中的脚本类不存在，就将文本颜色改为红色
				if (!scriptClassExists)
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f));
				
				// 将输入框中的内容赋值给脚本组件的ClassName
				if (ImGui::InputText("Class", buffer, sizeof(buffer)))
					component.ClassName = buffer;

				// 通过实体ID获取脚本实例
				Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity.GetUUID());
				
				// 暂时只支持运行时获取属性值，因为scriptInstance只有当运行时才不为空
				if (scriptInstance)
				{
					// 获取脚本中的属性名和信息
					const auto& fields = scriptInstance->GetScriptClass()->GetFields();
					for (const auto& [name, field] : fields)
					{
						// 如果是属性类型是Float，就提供一个拖动条，并允许设置属性的值，实现热更新
						if (field.Type == ScriptFieldType::Float)
						{
							// 获取属性的值
							float data = scriptInstance->GetFieldValue<float>(name);
							if (ImGui::DragFloat(name.c_str(), &data))
							{
								scriptInstance->SetFieldValue(name, data);
							}
						}
					}
				}

				if (!scriptClassExists)
					ImGui::PopStyleColor();
			});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
			{
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));

				ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
				// 在texture按钮上，接受拖动过来的值
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path texturePath = std::filesystem::path(g_AssetPath) / path;
						Ref<Texture2D> texture = Texture2D::Create(texturePath.string());

						if (texture->IsLoaded())
							component.Texture = texture;
						else
							HZ_WARN("Could not load texture {0}", texturePath.filename().string());
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
			});
		DrawComponent<MeshFilterComponent>("Mesh Filter", entity, [](auto& component)
			{
				const char* meshTypeStrings[] = { "StaticBatchable", "StaticUnique", "SkinnedMesh" };
				ImGui::Text("MeshType: ");
				ImGui::SameLine();
				ImGui::Text(meshTypeStrings[(int)component.MeshObj->GetMeshType()]);

				auto gtype = component.GType;
				const char* geometryTypeStrings[] = { "Cube", "Sphere", "Custom" };
				const char* currentMeshTypeString = geometryTypeStrings[(int)gtype];
				if (ImGui::BeginCombo("Geometry", currentMeshTypeString))
				{
					for (int i = 0; i < 3; i++)
					{
						bool isSelected = currentMeshTypeString == geometryTypeStrings[i];
						if (ImGui::Selectable(geometryTypeStrings[i], isSelected))
						{
							currentMeshTypeString = geometryTypeStrings[i];
							component.SetType((MeshFilterComponent::GeometryType)i);
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			});
		DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [](auto& component)
			{
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));

				ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
				// 在texture按钮上，接受拖动过来的值
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path texturePath = std::filesystem::path(g_AssetPath) / path;
						Ref<Texture2D> texture = Texture2D::Create(texturePath.string());

						if (texture->IsLoaded())
							component.Texture = texture;
						else
							HZ_WARN("Could not load texture {0}", texturePath.filename().string());
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
			});

		DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component)
			{
				const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
				const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];
				if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
				{
					for (int i = 0; i < 3; i++)
					{
						bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
						if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
						{
							currentBodyTypeString = bodyTypeStrings[i];
							component.Type = (Rigidbody2DComponent::BodyType)i;
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
			});

		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component)
			{
				ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
				ImGui::DragFloat2("Size", glm::value_ptr(component.Size));
				ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
			});
	}

	template<typename T>
	void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName) 
	{
		if (!m_SelectionContext.HasComponent<T>())
		{
			if (ImGui::MenuItem(entryName.c_str()))
			{
				m_SelectionContext.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}
}