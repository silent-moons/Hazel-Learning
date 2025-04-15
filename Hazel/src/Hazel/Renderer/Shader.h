#pragma once

#include <string>

#include <glm/glm.hpp>

namespace Hazel 
{
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

		static Ref<Shader> Create(const std::string& filepath);
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
	};

	class ShaderLibrary	//�������Ƚϸߣ�����ֱ���� Shader.h �ж��壬���漰ĳһ��ͼ�νӿڵ�ϸ��
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