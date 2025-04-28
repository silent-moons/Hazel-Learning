#pragma once

#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Texture.h"

#include <filesystem>
#include <yaml-cpp/yaml.h>

namespace Hazel
{
	static const std::filesystem::path AssetPath = "assets";

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
		std::unordered_map<std::string, ShaderPropertyType> AttribAndType;
		std::unordered_map<std::string, glm::vec4> AttribAndValue;
	private:
		std::string m_Path;
		Ref<Shader> m_Shader;
		std::vector<Ref<Texture2D>> m_Textures;
	};
}

namespace YAML
{
	template<>
	struct convert<Hazel::Ref<Hazel::Material>>
	{
		static Node encode(const Hazel::Material& mat)
		{
			Node node;
			node["shader"] = mat.GetShader()->GetName();
			Node texturePaths;
			for (const auto& texture : mat.GetTextures())
				texturePaths.push_back(texture->GetPath());
			node["textures"] = texturePaths;
			return node;
		}

		static bool decode(const Node& node, Hazel::Ref<Hazel::Material>& mat)
		{
			if (!node["shader"])
				return false;

			mat = Hazel::CreateRef<Hazel::Material>();
			std::string shaderName = node["shader"].as<std::string>();
			if (Hazel::ShaderLibrary::Exists(shaderName))
				mat->SetShader(Hazel::ShaderLibrary::Get(shaderName));
			else
			{
				std::string shaderPath = Hazel::AssetPath.string() + "/shaders/" + shaderName + ".glsl";
				mat->SetShader(Hazel::ShaderLibrary::Load(shaderPath));
			}
			mat->RegisterShaderProperty();
			for (const auto& texturePath : node["textures"])
				mat->AddTexture(Hazel::Texture2D::LoadCompressedFile(texturePath.as<std::string>()));
			return true;
		}
	};
}