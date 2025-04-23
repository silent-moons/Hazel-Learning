#include "hzpch.h"
#include "OpenGLTexture.h"

#include <fstream>
#include <glad/glad.h>
#include <glad/glext.h>
#include <stb_image.h>
#include <libsquish/squish.h>

namespace Hazel 
{
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, 
		int channels, bool internalCompress)
		: m_Width(width), m_Height(height), m_Channels(channels)
	{
		if (channels == 4)
		{
			m_InternalFormat = internalCompress ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_RGBA8;
			m_DataFormat = internalCompress ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_RGBA;
		}
		else
		{
			m_InternalFormat = internalCompress ? GL_COMPRESSED_RGB_S3TC_DXT1_EXT : GL_RGB8;
			m_DataFormat = internalCompress ? GL_COMPRESSED_RGB_S3TC_DXT1_EXT : GL_RGB;
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);	

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path) : m_Path(path)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		if (data)
		{
			m_IsLoaded = true;

			m_Width = width;
			m_Height = height;
			m_Channels = channels;
			GLenum internalFormat = 0, dataFormat = 0;
			if (channels == 4)
			{
				internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				dataFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			}
			else if (channels == 3)
			{
				internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
				dataFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			}
			m_InternalFormat = internalFormat;
			m_DataFormat = dataFormat;
			HZ_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");

			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
			glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);
			
			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
			
			int flag = 0;
			if (m_Channels == 4)
				flag = squish::kDxt5;
			else if (m_Channels == 3)
				flag = squish::kDxt1;
			int compressedSize = squish::GetStorageRequirements(m_Width, m_Height, flag);
			m_CompressedData.resize(compressedSize);
			squish::CompressImage(
				data, m_Width, m_Height,
				m_CompressedData.data(),
				flag | squish::kColourRangeFit // 暂时使用kColourRangeFit快速压缩
			);
			glCompressedTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, compressedSize, m_CompressedData.data());
			stbi_image_free(data);
		}
		else
			HZ_CORE_ERROR("Failed to load image: {0}", path);
		stbi_set_flip_vertically_on_load(0);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data)
	{
		int compressedSize = squish::GetStorageRequirements(m_Width, m_Height, squish::kDxt5);
		m_CompressedData.resize(compressedSize);
		squish::CompressImage(
			(unsigned char*)data, m_Width, m_Height,
			m_CompressedData.data(),
			squish::kDxt5 | squish::kColourRangeFit // 暂时使用kColourRangeFit快速压缩
		);
		glCompressedTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, compressedSize, m_CompressedData.data());
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		uint32_t bpp = (m_DataFormat == GL_RGBA ? 4 : 3);
		HZ_CORE_ASSERT((size == m_Width * m_Height * bpp), "Data must contain the full texture! Please check that the size of the data matches the format of the data");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::SetCompressedData(void* data, uint32_t size)
	{
		m_IsLoaded = true;
		glCompressedTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, size, data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLTexture2D::Export(const std::string& path)
	{
		m_MetaDataFilePath = path;

		TexFileHead texFileHead;
		texFileHead.MipmapLevel = 0;
		texFileHead.Width = m_Width;
		texFileHead.Height = m_Height;
		texFileHead.Channels = m_Channels;
		texFileHead.DataSize = m_CompressedData.size();
		std::ofstream outputStream(path, std::ios::out | std::ios::binary);

		outputStream.write((char*)&texFileHead, sizeof(texFileHead));
		outputStream.write((char*)m_CompressedData.data(), texFileHead.DataSize);
		outputStream.close();
		m_CompressedData.clear();
	}
}