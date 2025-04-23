#pragma once

#include <string>

namespace Hazel 
{
	struct TexFileHead
	{
		int MipmapLevel;
		uint32_t Width;
		uint32_t Height;
		int Channels;
		uint32_t DataSize;
	};

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual const std::string& GetPath() const = 0;
		virtual void SetPath(const std::string& path) = 0;
		virtual void SetData(void* data) = 0;
		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void SetCompressedData(void* data, uint32_t size) = 0;
		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual bool IsLoaded() const = 0;
		virtual bool operator==(const Texture& other) const = 0;
		virtual void Export(const std::string& path) = 0;
		virtual void SetMetaDataFilePath(const std::string& path) = 0;
		virtual std::string GetMetaDataFilePath() const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height, 
			int channels = 4, bool internalCompress = false);
		static Ref<Texture2D> Create(const std::string& path);
		static Ref<Texture2D> LoadCompressedFile(const std::string& path);
	};
}