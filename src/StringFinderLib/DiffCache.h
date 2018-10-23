#pragma once
#include "ICache.h"
#include "DiffCacheTypes.h"

namespace sf::lib
{
    class DiffCache : public ICache
    {
    public:
        explicit DiffCache(Data cacheData);
        
        const Data& GetCacheData() const override;

        // reset inner cache
        void Reset(Data cacheData) override;

        // return first possible match result with passed data
        std::optional<Result> GetFirstResult(size_t cmpDataOffset, const Data& cmpData) const override;

        // return next possible match result with passed data after previous result
        std::optional<Result> GetNextResult(const Result& prevRes, const Data& cmpData) const override;

    private:
        void ConstructCache();
        std::optional<Result> CompareWithCacheData(size_t cacheDataOffset, 
            size_t cmpDataOffset, 
            const Data& cmpData, 
            size_t cachedMatchLen = 0) const;

    private:
        Data m_cacheData;
        DiffCacheContainer m_cache;
        std::vector<DiffCacheContainer::iterator> m_iteratorList;
    };
}