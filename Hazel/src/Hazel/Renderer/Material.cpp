#include "hzpch.h"

#include "Material.h"

#include <fstream>

namespace Hazel
{
    Material::Material(const std::string& path) : m_Path(path)
    {
		YAML::Node node;
		try
		{
			node = YAML::LoadFile(path);
		}
		catch (YAML::ParserException e)
		{
			HZ_CORE_ERROR("Failed to load .mat file '{0}'\n{1}", path, e.what());
		}
		std::string shaderName = node["shader"].as<std::string>();
		if (ShaderLibrary::Exists(shaderName))
			m_Shader = ShaderLibrary::Get(shaderName);
		else
		{
			std::string shaderPath = AssetPath.string() + "/shaders/" + shaderName + ".glsl";
			m_Shader = ShaderLibrary::Load(shaderPath);
		}
		RegisterShaderProperty();
		for (const auto& texturePath : node["textures"])
			AddTexture(Hazel::Texture2D::LoadCompressedFile(texturePath.as<std::string>()));
    }

	void Material::RegisterShaderProperty()
	{
		if (!m_Shader)
		{
			HZ_CORE_WARN("Material has no shader, register failed.");
			return;
		}
		AttribAndType = m_Shader->GetAttribAndType();
		AttribAndValue = m_Shader->GetAttribAndValue();
	}

	void Material::Export(const std::string& path)
	{
		YAML::Node node;
		node["shader"] = m_Shader->GetName();
		for (const auto& texture : m_Textures)
			node["textures"].push_back(texture->GetMetaDataFilePath());
		std::ofstream fout(path);
		fout << node;
	}
}