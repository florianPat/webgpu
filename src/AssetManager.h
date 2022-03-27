#pragma once

#include <unordered_map>
#include <malloc.h>
#include "AssetLoader.h"
#include "Utils.h"
#include <intrin.h>

class AssetManager
{
    struct FilenameCacheValue
    {
        uint32_t ressourceCacheAssetId;
        //NOTE: The pointers do not get invalidated in shrink/expand because I just store a pp
        int8_t* asset;
    };

    struct RessourceCacheAssetVector
    {
        const String& filename;
		int8_t* assetP;
    };
private:
	static constexpr uint64_t maxSize = Gigabyte(1);
	uint64_t currentSize = 0;
	std::unordered_map<String, FilenameCacheValue> filenameCache;
	std::unordered_map<String, int32_t> assetLoaderCache;
	Vector<std::pair<AssetLoader, Vector<RessourceCacheAssetVector>>> ressourceCache;
	volatile uint32_t mutex = 0;
public:
    AssetManager() = default;
	template <typename T, typename... Args>
	T* getOrAddRes(const String& filename, Args&&... args);
	bool unloadNotUsedRes(const String& filename);
	void clear();
	bool isLoaded(const String& filename);
	void reloadAllRes();
	void registerAssetLoader(const String& fileExt, const AssetLoader& assetLoader);
};

template<typename T, typename... Args>
T * AssetManager::getOrAddRes(const String & filename, Args&&... args)
{
	auto res = filenameCache.find(filename);
	if (res != filenameCache.end())
	{
		T* asset = (T*) res->second.asset;
		return asset;
	}
	else
	{
		auto asset = (int8_t*) malloc(sizeof(T));
		auto tP = (T*) asset;
		new (tP) T(filename, std::forward<Args>(args)...);
		String ext = filename.substr(filename.length() - 3);
		assert(assetLoaderCache.find(ext) != assetLoaderCache.end());
		int32_t assetLoaderIndex = assetLoaderCache.at(ext);

		auto& ressourceCachePair = ressourceCache.at(assetLoaderIndex);

		currentSize += tP->getSize();
		//TODO: Do something if the assetCache is full!
		if (currentSize > maxSize)
		{
			InvalidCodePath;
//			do
//			{
//				auto id = timeOfInsertCache.begin();
//				auto it = ressourceCache.find(*id);
//				assert(it != ressourceCache.end());
//				AssetLoader aL = assetLoaderCache.at(it->first.substr(it->first.length() - 3));
//				currentSize -= aL.getSize(it->second.get());
//				it->second.release();
//				ressourceCache.erase(it);
//				timeOfInsertCache.erase(id);
//			} while (currentSize > maxSize);
		}

		uint32_t readValue = 1;
		do {
			readValue = InterlockedCompareExchange(&mutex, 1, 0);
		} while (readValue == 0);

		// TODO: Lock free data structure!
        auto result = filenameCache.emplace(std::make_pair(filename,
                FilenameCacheValue{ ressourceCachePair.second.size(), asset} ));
        assert(result.second);

		ressourceCachePair.second.push_back(RessourceCacheAssetVector{ result.first->first, asset });

		_WriteBarrier();
		_mm_sfence();

		mutex = 0;

		T* returnAsset = (T*) asset;
		return returnAsset;
	}
}