#include "hzpch.h"

#include "Model.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Math/Math.h"

#include <stb_image.h>

namespace Hazel
{
	std::unordered_map<std::string, Ref<Texture2D>> Model::s_TextureCache;

	void Model::Read(const std::string& path, const Ref<Scene>& context)
	{
		std::size_t lastIndex = path.find_last_of('/');
		if (lastIndex == std::string::npos) // 适应相对路径和绝对路径
			lastIndex = path.find_last_of('\\');
		std::string rootPath = path.substr(0, lastIndex + 1);

		//开始进行读取
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			path,
			aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices
			| aiProcess_CalcTangentSpace | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			HZ_CORE_ERROR("Error:model read fail!");
			return;
		}

		ProcessNode({entt::null, nullptr}, scene->mRootNode, scene, context, rootPath);
	}

	void Model::ProcessNode(Entity parent, aiNode* ainode, const aiScene* scene, 
		const Ref<Scene>& context, const std::string& rootPath)
	{
		//创建新节点
		Entity node = context->CreateEntity(ainode->mName.C_Str());
		auto& trans = node.GetComponent<TransformComponent>();
		if (parent)
		{
			auto& parentTrans = parent.GetComponent<TransformComponent>();
			trans.Parent = parent.GetUUID();
			parentTrans.Children.push_back(node.GetUUID());
			glm::vec3 translation, rotation, scale;
			Math::DecomposeTransform(GetMat4f(ainode->mTransformation), translation, rotation, scale);
			trans.Translation = translation;
			trans.Rotation = rotation;
			trans.Scale = scale;
			trans.WorldTransform = parentTrans.WorldTransform * trans.LocalTransform();
		}

		// 处理节点的网格，如果节点有1个网格，那么将节点本身加上Mesh相关组件
		if (ainode->mNumMeshes == 1)
		{
			aiMesh* aimesh = scene->mMeshes[ainode->mMeshes[0]];
			Ref<Mesh> mesh = ProcessMesh(aimesh, scene, rootPath);
			auto& mfc = node.AddComponent<MeshFilterComponent>();
			mfc.GType = MeshFilterComponent::GeometryType::Custom;
			mfc.MeshObj = mesh;
			node.AddComponent<MeshRendererComponent>();
		}
		// 如果节点有1个以上网格，就创建子节点
		else
		{
			for (unsigned int i = 0; i < ainode->mNumMeshes; i++)
			{
				aiMesh* aimesh = scene->mMeshes[ainode->mMeshes[i]];
				Ref<Mesh> mesh = ProcessMesh(aimesh, scene, rootPath);
				Entity subMesh = context->CreateEntity(aimesh->mName.C_Str());
				auto& mfc = subMesh.AddComponent<MeshFilterComponent>();
				mfc.GType = MeshFilterComponent::GeometryType::Custom;
				mfc.MeshObj = mesh;
				subMesh.AddComponent<MeshRendererComponent>();
				trans.Children.push_back(subMesh.GetUUID());
				subMesh.GetComponent<TransformComponent>().Parent = node.GetUUID();
			}
		}

		// 接下来对它的子节点重复这一过程
		for (unsigned int i = 0; i < ainode->mNumChildren; i++)
			ProcessNode(node, ainode->mChildren[i], scene, context, rootPath);
	}

	Ref<Mesh> Model::ProcessMesh(aiMesh* aimesh, const aiScene* scene, const std::string& rootPath)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<Ref<Texture2D>> textures;
		for (unsigned int i = 0; i < aimesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;
			vector.x = aimesh->mVertices[i].x;
			vector.y = aimesh->mVertices[i].y;
			vector.z = aimesh->mVertices[i].z;
			vertex.Position = vector;

			if (aimesh->HasNormals())
			{
				vector.x = aimesh->mNormals[i].x;
				vector.y = aimesh->mNormals[i].y;
				vector.z = aimesh->mNormals[i].z;
				vertex.Normal = vector;
			}

			if (aimesh->mTextureCoords[0]) // 颜色纹理
			{
				glm::vec2 vec;
				vec.x = aimesh->mTextureCoords[0][i].x;
				vec.y = aimesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
				// tangent
				vector.x = aimesh->mTangents[i].x;
				vector.y = aimesh->mTangents[i].y;
				vector.z = aimesh->mTangents[i].z;
				vertex.Tangent = vector;
				// bitangent
				vector.x = aimesh->mBitangents[i].x;
				vector.y = aimesh->mBitangents[i].y;
				vector.z = aimesh->mBitangents[i].z;
				vertex.Bitangent = vector;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);

			vertices.emplace_back(vertex);
		}
		// 处理索引
		for (unsigned int i = 0; i < aimesh->mNumFaces; i++)
		{
			aiFace face = aimesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.emplace_back(face.mIndices[j]);
		}
		if (aimesh->mMaterialIndex >= 0)
		{
			//取出材质
			aiMaterial* material = scene->mMaterials[aimesh->mMaterialIndex];
			textures.emplace_back(ProcessTexture(material, aiTextureType_DIFFUSE, 
				scene, rootPath));
		}

		return CreateRef<UniqueMesh>(vertices, indices, textures);
	}

	Ref<Texture2D> Model::ProcessTexture(
		const aiMaterial* material,
		const aiTextureType& type,
		const aiScene* scene,
		const std::string& rootPath)
	{
		aiString aiPath;
		material->Get(AI_MATKEY_TEXTURE(type, 0), aiPath);
		if (!aiPath.length)
			return 0;
		
		//先检查缓存是否有纹理
		auto iter = s_TextureCache.find(std::string(aiPath.C_Str()));
		if (iter != s_TextureCache.end())
			return iter->second;

		//部分模型在导出的时候，会把纹理图片打包到比如fbx格式当中，被打包到模型里面的图片，称为embeddedTexture 
		const aiTexture* assimpTexture = scene->GetEmbeddedTexture(aiPath.C_Str());
		Ref<Texture2D> tex;
		if (assimpTexture)
		{
			//如果确实图片打包在了模型内部，则上述代码获取到的aiTexture里面就含有了图片数据
			unsigned char* dataIn = reinterpret_cast<unsigned char*>(assimpTexture->pcData);
			uint32_t widthIn = assimpTexture->mWidth;
			uint32_t heightIn = assimpTexture->mHeight;
			uint32_t dataInSize = 0;
			if (!heightIn)
				dataInSize = widthIn;
			else
				dataInSize = widthIn * heightIn;
			int width = 0, height = 0, picType = 0;
			unsigned char* bits = stbi_load_from_memory(dataIn, dataInSize, &width, &height, &picType, STBI_rgb_alpha);
			std::string path = aiPath.C_Str();
			tex = Texture2D::Create(width, height);
			tex->SetData(bits, width * height * 4);
			tex->SetPath(path);
		}
		else
		{
			std::string fullPath = rootPath + aiPath.C_Str();
			tex = Texture2D::Create(fullPath);
		}
		s_TextureCache.emplace(aiPath.C_Str(), tex);

		return tex;
	}

	glm::mat4 Model::GetMat4f(aiMatrix4x4 value)
	{
		glm::mat4 to(
			value.a1, value.a2, value.a3, value.a4,
			value.b1, value.b2, value.b3, value.b4,
			value.c1, value.c2, value.c3, value.c4,
			value.d1, value.d2, value.d3, value.d4
		);

		return to;
	}
}