#include "hzpch.h"

#include "Material.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Hazel
{
    Material::Material(const std::string& path) : m_Path(path) {}

	void Material::RegisterShaderProperty()
	{
		if (!m_Shader)
		{
			HZ_CORE_WARN("Material has no shader, register failed.");
			return;
		}
		AttribInfo = m_Shader->GetAttribInfo();
	}

	void Material::Export(const std::string& path)
	{
		YAML::Node node;
		node["shader"] = m_Shader->GetName();
		for (const auto& texture : m_Textures)
			node["textures"].push_back(texture->GetMetaDataFilePath());
		YAML::Node properties;
		for (const auto& [attrib, typeAndValue] : AttribInfo)
		{
			YAML::Node nameTypeValue;
			nameTypeValue["name"] = attrib;
			nameTypeValue["type"] = ShaderPropertyTypeToString(typeAndValue.first);
			nameTypeValue["value"].push_back(typeAndValue.second.x);
			nameTypeValue["value"].push_back(typeAndValue.second.y);
			nameTypeValue["value"].push_back(typeAndValue.second.z);
			nameTypeValue["value"].push_back(typeAndValue.second.w);
			nameTypeValue["value"].SetStyle(YAML::EmitterStyle::Flow);
			properties.push_back(nameTypeValue);
		}
		node["properties"] = properties;
		std::ofstream fout(path);
		fout << node;
	}

	void Material::ProcessCache(AssetCache<Texture2D>& texture2DCache)
	{
		YAML::Node node;
		try
		{
			node = YAML::LoadFile(m_Path);
		}
		catch (YAML::ParserException e)
		{
			HZ_CORE_ERROR("Decode function failed to load .mat file '{0}'\n{1}", m_Path, e.what());
		}
		std::string shaderName = node["shader"].as<std::string>();
		if (ShaderLibrary::Exists(shaderName))
			m_Shader = ShaderLibrary::Get(shaderName);
		else
		{
			std::string shaderPath = g_AssetPath.string() + "/shaders/" + shaderName + ".glsl";
			m_Shader = ShaderLibrary::Load(shaderPath);
		}
		RegisterShaderProperty();
		for (const auto& texturePath : node["textures"])
			AddTexture(texture2DCache.Load(texturePath.as<std::string>()));
		for (const auto& property : node["properties"])
		{
			std::string attrib = property["name"].as<std::string>();
			std::string typeStr = property["type"].as<std::string>();
			ShaderPropertyType type = ShaderPropertyTypeFromString(typeStr);
			glm::vec4 value;
			value.x = property["value"][0].as<float>();
			value.y = property["value"][1].as<float>();
			value.z = property["value"][2].as<float>();
			value.w = property["value"][3].as<float>();
			AttribInfo[attrib] = { type, value };
		}
	}

	Ref<Material> Material::Create(const std::string& path)
	{
		return CreateRef<Material>(path);
	}
}