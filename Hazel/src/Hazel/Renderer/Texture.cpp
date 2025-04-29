#include "hzpch.h"

#include <fstream>

#include "Texture.h"
#include "Hazel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Hazel 
{
	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height, 
		int channels, bool internalCompress)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: 
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported! ")
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(width, height, channels, internalCompress);
		case RendererAPI::API::DirectX: 
			HZ_CORE_ASSERT(false, "RendererAPI::DirectX is currently not supported! ")
			return nullptr;
		}
		HZ_CORE_ASSERT(false, "Unknown Renderer API!")
			return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		if (path.substr(path.find_last_of('.') + 1) == "cpt")
			return Texture2D::LoadCompressedFile(path);

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: 
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported! ")
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(path);
		case RendererAPI::API::DirectX: 
			HZ_CORE_ASSERT(false, "RendererAPI::DirectX is currently not supported! ")
			return nullptr;
		}

		HZ_CORE_ASSERT(false, "Unknown Renderer API!")
			return nullptr;
	}

	Ref<Texture2D> Texture2D::LoadCompressedFile(const std::string& path)
	{
		if (path.substr(path.find_last_of('.') + 1) != "cpt")
		{
			HZ_CORE_WARN("Not a cpt file, creating a non-compressed texture.");
			return Texture2D::Create(path);
		}
		std::ifstream inputTexStream(path, std::ios::in | std::ios::binary);
		TexFileHead texFileHead;
		inputTexStream.read((char*)&texFileHead, sizeof(texFileHead));
		unsigned char* data = new unsigned char[texFileHead.DataSize];
		inputTexStream.read((char*)data, texFileHead.DataSize);
		Ref<Texture2D> texture = Texture2D::Create(texFileHead.Width, 
			texFileHead.Height, texFileHead.Channels, true);
		texture->SetCompressedData(data, texFileHead.DataSize);
		texture->SetMetaDataFilePath(path);
		texture->SetPath(path);
		delete[] data;
		inputTexStream.close();
		return texture;
	}
}