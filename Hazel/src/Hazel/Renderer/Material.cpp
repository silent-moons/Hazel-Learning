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
			SetShader(ShaderLibrary::Get(shaderName));
		else
		{
			std::string shaderPath = AssetPath.string() + "/shaders/" + shaderName + ".glsl";
			SetShader(ShaderLibrary::Load(shaderPath));
		}
		
		for (const auto& texturePath : node["textures"])
			AddTexture(Hazel::Texture2D::LoadCompressedFile(texturePath.as<std::string>()));
    }

	void Material::Export(const std::string& path)
	{
		YAML::Node node;
		node["shader"] = GetShader()->GetName();
		for (const auto& texture : GetTextures())
			node["textures"].push_back(texture->GetMetaDataFilePath());
		std::ofstream fout(path);
		fout << node;
	}
}