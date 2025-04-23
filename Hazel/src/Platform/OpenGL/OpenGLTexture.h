#pragma once

#include "Hazel/Renderer/Texture.h"

#include <glad/glad.h>

namespace Hazel 
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height, 
			int channels = 4, bool internalCompress = false);
		OpenGLTexture2D(const std::string& path);
		~OpenGLTexture2D();

		uint32_t GetWidth() const override { return m_Width; }
		uint32_t GetHeight() const override { return m_Height; }
		uint32_t GetRendererID() const override { return m_RendererID; }
		const std::string& GetPath() const override { return m_Path; }
		void SetPath(const std::string& path) override { m_Path = path; }
		void SetData(void* data) override;
		void SetData(void* data, uint32_t size) override;
		void SetCompressedData(void* data, uint32_t size) override;
		void Bind(uint32_t slot = 0) const override;
		bool IsLoaded() const override { return m_IsLoaded; }
		bool operator==(const Texture& other) const override
		{
			return m_RendererID == other.GetRendererID();
		}
		void Export(const std::string& path) override;
		void SetMetaDataFilePath(const std::string& path) override { m_MetaDataFilePath = path; }
		std::string GetMetaDataFilePath() const override { return m_MetaDataFilePath; }

	private:
		std::string m_Path;
		std::string m_MetaDataFilePath;
		bool m_IsLoaded = false;

		uint32_t m_Width;
		uint32_t m_Height;
		int m_Channels;
		uint32_t m_RendererID;
		std::vector<uint8_t> m_CompressedData;
		GLenum m_InternalFormat, m_DataFormat;
	};

}