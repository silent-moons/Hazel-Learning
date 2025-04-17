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
				
				// �ڳ��������ڵ��»���ʵ��
				m_Context->m_Registry.view<TransformComponent>().each(
					[&](auto entityID, TransformComponent& tf)
					{
						Entity entity{ entityID , m_Context.get() };
						if (tf.Parent == 0)
							DrawEntityNode(entity);
					});
				ImGui::TreePop();
			}

			// ��������ڿհ״�������ǰѡ���ʵ����Ϊ�գ��·���������Ⱦʵ���Ӧ�����
			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				m_SelectionContext = {};

			// ����һ�������������ڵ� InvisibleButton ��Ϊ��������
			ImVec2 winSize = ImGui::GetContentRegionAvail();
			ImVec2 winPos = ImGui::GetCursorScreenPos();
			ImGui::SetCursorScreenPos(winPos); // ���ù��λ��
			ImGui::InvisibleButton("##DropZone", winSize);
			
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				ImGui::OpenPopup("HierarchyContextMenu");
			if (ImGui::BeginPopup("HierarchyContextMenu"))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
					m_Context->CreateEntity("Empty Entity");
				ImGui::EndPopup();
			}

			// ��קʵ�嵽�հ״�
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
		// ��ֹ���ӿڵ�ѡʵ��ʱ��������û��ʧ��
		// ���û�������룬��ѡʵ��ʱ���������ĳ���ؼ��ڼ���״̬������tag�����
		// ��ô��ѡ���ʵ��tag�ͻ�����һ��ʵ���tag����Ϊ��������ͬ�Ŀؼ�ID
		// ���Ա��뱣֤ÿ�ε�ѡʵ��ʱ��������Ҫʧ��
		// �ڲ㼶����е�ѡʵ��ʱ���㼶�����Զ��۽����������Զ�ʧ��������û����������
		ImGui::SetWindowFocus("Scene Hierarchy");
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		auto& transform = entity.GetComponent<TransformComponent>();
		bool hasChildren = !transform.Children.empty();

		// �ڵ��Ƿ�ѡ�У��ڵ��ͷ�Ƿ��Ǵ�״̬
		ImGuiTreeNodeFlags flags = (m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0;
		flags |= ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		if (!hasChildren)
			flags |= ImGuiTreeNodeFlags_Leaf;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
			m_SelectionContext = entity;

		// ��ק��ʼ
		if (ImGui::BeginDragDropSource()) 
		{
			m_DragDropSource = entity;
			ImGui::SetDragDropPayload("ENTITY", &m_DragDropSource, sizeof(Entity));
			ImGui::Text("Set Parent");
			ImGui::EndDragDropSource();
		}

		// ��קĿ��
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

		// �����β˵�����Ŀ���һ�������ɾ���˵�
		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;
			ImGui::EndPopup();
		}

		if (opened)
		{
			// �ݹ����ʵ����ӽڵ�
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

		// ��Ϊ���У����ΪTranslation/rotation/scale���ұ�Ϊ����ֵ
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth); // ���õ�0�е��п�
		ImGui::Text(label.c_str());
		ImGui::NextColumn(); // �л�����һ��

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth()); // ����3��item����̬�������
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 }); // ��Ŀ֮���ˮƽ��ֱ��඼��Ϊ0

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize)) // �����ť������X��ֵ
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine(); // ����һ��item����ͬһ��
		// ʹ��##�����ָ��ؼ�����ʾ��ǩ�����ʶ����
		// ���磬"Button##UniqueID"�У�"Button"���û��ڽ����Ͽ������ı�����UniqueID�� ImGui �������ٸÿؼ����ڲ���ʶ����
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"); // �����϶��ı�X��ֵ
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
		
		// ʵ��ӵ�и����ʱ�ٻ�
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
				ImGui::OpenPopup("ComponentSettings"); // +�Ű�ť�ĵ����˵�
			}
			bool removeComponent = false;
			// Ŀǰ�˵�����ѡ��ɾ�����
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;
				ImGui::EndPopup();
			}
			if (open) // �������˵��Ǵ�״̬���ͻص�����Ĳ�ͬ�����ר�û��ƺ���
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

		ImGui::SameLine(); // �������İ�ť����ͬһ��
		ImGui::PushItemWidth(-1);
		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent"); // �����˵�
		if (ImGui::BeginPopup("AddComponent")) // ����������
		{
			// TODO: ������������ӣ�����᲻���޸ģ����Ǹ��õ����
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

		// TODO: ���������ƣ�ÿ�¶���һ���������Ҫ��������ӻ��Ƶľ����߼�
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

		// lambda����ı���Ĭ��Ϊconst�����Ը�lambda����Ϊmutable���Ѳ���ı���ȥ��const
		DrawComponent<ScriptComponent>("Script", entity, [entity](auto& component) mutable
			{
				bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);
				static char buffer[64];
				strcpy(buffer, component.ClassName.c_str());
				// ���������еĽű��಻���ڣ��ͽ��ı���ɫ��Ϊ��ɫ
				if (!scriptClassExists)
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f));
				
				// ��������е����ݸ�ֵ���ű������ClassName
				if (ImGui::InputText("Class", buffer, sizeof(buffer)))
					component.ClassName = buffer;

				// ͨ��ʵ��ID��ȡ�ű�ʵ��
				Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity.GetUUID());
				
				// ��ʱֻ֧������ʱ��ȡ����ֵ����ΪscriptInstanceֻ�е�����ʱ�Ų�Ϊ��
				if (scriptInstance)
				{
					// ��ȡ�ű��е�����������Ϣ
					const auto& fields = scriptInstance->GetScriptClass()->GetFields();
					for (const auto& [name, field] : fields)
					{
						// ���������������Float�����ṩһ���϶������������������Ե�ֵ��ʵ���ȸ���
						if (field.Type == ScriptFieldType::Float)
						{
							// ��ȡ���Ե�ֵ
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
				// ��texture��ť�ϣ������϶�������ֵ
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
				// ��texture��ť�ϣ������϶�������ֵ
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