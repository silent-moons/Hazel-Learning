#pragma once

#include "Base.h"

namespace Hazel
{
    template<typename T>
    class AssetCache
    {
    public:
        AssetCache() = default;
        AssetCache(const AssetCache&) = delete;
        AssetCache& operator=(const AssetCache&) = delete;
        Ref<T> Load(const std::string& path)
        {
            if (m_Cache.find(path) != m_Cache.end())
                return m_Cache[path];
            auto asset = T::Create(path);
            m_Cache[path] = asset;
            return asset;
        }

        bool Exists(const std::string& path)
        {
            return m_Cache.find(path) != m_Cache.end();
        }

		void Set(const std::string& path, const Ref<T>& asset)
		{
			m_Cache[path] = asset;
		}

        Ref<T> Get(const std::string& path)
        {
            if (!Exists(path))
            {
                HZ_CORE_ERROR("Asset {} not found!", path);
				return nullptr;
            }
            return m_Cache[path];
        }

    private:
        std::unordered_map<std::string, Ref<T>> m_Cache;
    };
}
