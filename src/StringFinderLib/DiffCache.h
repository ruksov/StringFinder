#pragma once
#include "ICache.h"
#include "DiffCacheTypes.h"

namespace sf::lib
{
    class DiffCache : public ICache
    {
    public:
        explicit DiffCache(Data cacheData);
        ~DiffCache();
        
        const Data& GetCacheData() const override;

        // reset inner cache
        void Reset(Data cacheData) override;

		// If inOutRes specified it will be updated to more suitable result,
		// else it will be filled first match result in diff tree.
		// This result will be used to try find next result in GetNextResult.
		// Method returns true if inOutRes changed and can be used.
        bool GetFirstResult(CacheMatchResult& inOutRes, const Data& cmpData) override;

        // return next possible match result with passed data after previous result
        bool GetNextResult(CacheMatchResult& inOutRes, const Data& cmpData) override;

    private:
        void ConstructCache();
        size_t CompareWithCacheData(size_t cacheDataOffset,
            size_t cmpDataOffset, 
            const Data& cmpData, 
            size_t cachedMatchLen = 0) const;

        // return first parent iterator in it diff cache tree,
        // which has diff offset less than match length
        DiffCacheContainer::iterator& FindHighestParent(size_t matchLen,
            DiffCacheContainer::iterator& it);

    private:
        Data m_cacheData;
        DiffCacheContainer m_cache;
        std::vector<DiffCacheContainer::iterator> m_iteratorList;
    };
}