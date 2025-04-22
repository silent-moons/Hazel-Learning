#pragma once

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Mesh.h"

namespace Hazel
{
	class Scene;
	class Entity;
	class Model
	{
	public:
		static void Read(const std::string& path, const Ref<Scene>& context);
	private:
		static void ProcessNode(
			Entity parent, 
			aiNode* ainode, 
			const aiScene* scene, 
			const Ref<Scene>& context,
			const std::string& rootPath);
		static Ref<Mesh> ProcessMesh(
			aiMesh* aimesh, 
			const aiScene* scene, 
			const std::string& rootPath);
		static Ref<Texture2D> ProcessTexture(
			const aiMaterial* material, 
			const aiTextureType& type, 
			const aiScene* scene, 
			const std::string& rootPath);
		static glm::mat4 GetMat4f(aiMatrix4x4 value);

		static std::unordered_map<std::string, Ref<Texture2D>> s_TextureCache;
	};
}