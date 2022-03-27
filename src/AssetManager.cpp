#include "AssetManager.h"
#include "Utils.h"

bool AssetManager::unloadNotUsedRes(const String & filename)
{
	auto res = filenameCache.find(filename);
	if (res != filenameCache.end())
	{
	    int32_t assetLoaderIndex = assetLoaderCache.at(res->first.substr(res->first.length() - 3));
		auto& ressourceCachePair = ressourceCache.at(assetLoaderIndex);
		AssetLoader& assetLoader = ressourceCachePair.first;

		currentSize -= assetLoader.getSize(res->second.asset);
		assetLoader.destruct(res->second.asset);
		free(res->second.asset);
		res->second.asset = nullptr;
		ressourceCachePair.second.erasePop_back(res->second.ressourceCacheAssetId);

		uint32_t ressourceCacheAssetIndex = res->second.ressourceCacheAssetId;
		for (auto it = ressourceCachePair.second.begin() + res->second.ressourceCacheAssetId; it != ressourceCachePair.second.end(); ++it, ++ressourceCacheAssetIndex)
		{
			auto movedRes = filenameCache.find(it->filename);
			assert(res != filenameCache.end());
			movedRes->second.ressourceCacheAssetId = ressourceCacheAssetIndex;
		}

        filenameCache.erase(res);
		return true;
	}
	else
		return false;
}

void AssetManager::clear()
{
	for (auto it = ressourceCache.begin(); it != ressourceCache.end(); ++it)
	{
		AssetLoader& assetLoader = it->first;

		for(auto assetIt = it->second.begin(); assetIt != it->second.end(); ++assetIt)
        {
            assetLoader.destruct(assetIt->assetP);
            free(assetIt->assetP);
            assetIt->assetP = nullptr;
            filenameCache.erase(assetIt->filename);
        }

		it->second.clear();
	}

	currentSize = 0;
}

bool AssetManager::isLoaded(const String & filename)
{
	auto i = filenameCache.find(filename);
	return i != filenameCache.end();
}

void AssetManager::reloadAllRes()
{
	for (auto it = ressourceCache.begin(); it != ressourceCache.end(); ++it)
	{
		AssetLoader& assetLoader = it->first;

		if(assetLoader.isGpu)
		{
		    for(auto assetIt = it->second.begin(); assetIt != it->second.end(); ++assetIt)
            {
                assetLoader.reloadFromFile(assetIt->assetP, assetIt->filename);
            }
		}
	}
}

void AssetManager::registerAssetLoader(const String & fileExt, const AssetLoader & assetLoader)
{
	assert(assetLoaderCache.find(fileExt) == assetLoaderCache.end());

    assetLoaderCache.emplace(std::make_pair(fileExt, ressourceCache.size()));
	ressourceCache.push_back(std::make_pair(assetLoader, Vector<RessourceCacheAssetVector>()));
}
