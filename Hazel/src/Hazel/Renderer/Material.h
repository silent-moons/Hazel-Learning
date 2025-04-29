#pragma once

#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Core/AssetCache.h"

#include <filesystem>

namespace Hazel
{
	extern const std::filesystem::path g_AssetPath;

	class Material
	{
	public:
		Material() = default;
		Material(const std::string& path);
		const std::string& GetPath() const { return m_Path; }
		void SetPath(const std::string& path) { m_Path = path; }
		const Ref<Shader>& GetShader() const { return m_Shader; }
		void SetShader(const Ref<Shader>& shader) { m_Shader = shader; }
		void RegisterShaderProperty();
		void AddTexture(const Ref<Texture2D>& texture) { m_Textures.push_back(texture); }
		const std::vector<Ref<Texture2D>>& GetTextures() const { return m_Textures; }
		std::vector<Ref<Texture2D>>& GetTextures() { return m_Textures; }

		void Export(const std::string& path);
		void ProcessCache(AssetCache<Texture2D>& texture2DCache);
		std::unordered_map<std::string, std::pair<ShaderPropertyType, glm::vec4>> AttribInfo;
		static Ref<Material> Create(const std::string& path);
	private:
		std::string m_Path;
		Ref<Shader> m_Shader;
		std::vector<Ref<Texture2D>> m_Textures;
	};
}