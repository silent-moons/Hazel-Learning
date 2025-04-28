#pragma once

#include <string>

#include <glm/glm.hpp>

namespace Hazel 
{
	enum class ShaderPropertyType
	{
		Float, Float2, Float3, Float4,
		Color3, Color4,
		Int, Int2, Int3, Int4
	};

	static ShaderPropertyType ShaderPropertyTypeFromString(const std::string& typeString)
	{
		if (typeString == "color4")
			return ShaderPropertyType::Color4;
		if (typeString == "float")
			return ShaderPropertyType::Float;

		HZ_CORE_ASSERT(false, "Unknown shader property type");
		return ShaderPropertyType::Float;
	}

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		virtual const std::string& GetName() const = 0;
		virtual const std::unordered_map<std::string, ShaderPropertyType>& GetAttribAndType() const = 0;
		virtual std::unordered_map<std::string, glm::vec4>& GetAttribAndValue() = 0;

		static Ref<Shader> Create(const std::string& filepath);
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
	};

	class ShaderLibrary	//此类抽象度较高，可以直接在 Shader.h 中定义，不涉及某一个图形接口的细节
	{
	public:
		ShaderLibrary() = delete;
		ShaderLibrary(const ShaderLibrary&) = delete;
		ShaderLibrary& operator=(const ShaderLibrary&) = delete;

		static void Add(const std::string& name, const Ref<Shader>& shader);
		static void Add(const Ref<Shader>& shader);
		static Ref<Shader> Load(const std::string& filepath);
		static Ref<Shader> Load(const std::string& name, const std::string& filepath);
		static Ref<Shader> Get(const std::string& name);
		static bool Exists(const std::string& name);
	private:
		static std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}