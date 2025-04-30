#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>

#include "Mesh.h"

namespace Hazel
{
	class Scene;
	class Entity;
	class Model
	{
	public:
		Model() = delete;
		static void Read(const std::string& path, const Ref<Scene>& context);
	private:
		static void ProcessNode(
			Entity parent, 
			aiNode* ainode, 
			const aiScene* scene, 
			const Ref<Scene>& context,
			const std::filesystem::path& dataRelativeDir);
		static std::pair<Ref<Mesh>, std::vector<Ref<Texture2D>>> ProcessMesh(
			Entity entity,
			aiMesh* aimesh, 
			const aiScene* scene, 
			const Ref<Scene>& context,
			const std::filesystem::path& dataRelativeDir);
		static Ref<Texture2D> ProcessTexture(
			Entity entity,
			const aiMaterial* material, 
			const aiTextureType& type, 
			const aiScene* scene, 
			const Ref<Scene>& context,
			const std::filesystem::path& dataRelativeDir);
		static glm::mat4 GetMat4f(aiMatrix4x4 value);
	};
}