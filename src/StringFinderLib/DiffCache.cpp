#include "stdafx.h"
#include "DiffCache.h"
#include "Exceptions.h"

namespace sf::lib
{
    DiffCache::DiffCache(Data cacheData)
        : m_cacheData(std::move(cacheData))
    {
        ConstructCache();
    }

    const Data & DiffCache::GetCacheData() const
    {
        return m_cacheData;
    }

    void DiffCache::Reset(Data cacheData)
    {
        m_cacheData = std::move(cacheData);
        ConstructCache();
    }

    std::optional<CacheMatchResult> DiffCache::GetFirstResult(size_t cmpDataOffset, 
        const Data & cmpData) const
    {
        if (cmpDataOffset >= cmpData.size())
        {
            return std::nullopt;
        }

        auto it = m_cache.find(DiffCacheKey(0, cmpData.at(cmpDataOffset)));
        if (it == m_cache.end())
        {
            return std::nullopt;
        }

        return CompareWithCacheData(it->second.Offset, cmpDataOffset, cmpData);
    }

    std::optional<CacheMatchResult> DiffCache::GetNextResult(const CacheMatchResult & prevRes, 
        const Data & cmpData) const
    {
        // try to update previous result with new cmp data
        auto updatedPrevRes = CompareWithCacheData(prevRes.CacheOffset
            , prevRes.CmpDataOffset
            , cmpData
            , prevRes.MatchLen);

        if (!updatedPrevRes)
        {
            updatedPrevRes = prevRes;
        }

        auto prevIt = m_iteratorList.at(updatedPrevRes->CacheOffset);

        if (prevIt->second.Offset != updatedPrevRes->CacheOffset    // range from prev result is sub range, which placed before
            || !prevIt->second.DiffRanges                           // range from prev result has not any diff sub ranges
            || updatedPrevRes->CmpDataOffset                        // end of cmp data range
                + updatedPrevRes->MatchLen >= cmpData.size())                   
        {
            return std::nullopt;
        }

        auto it = prevIt->second.DiffRanges->find(
            DiffCacheKey(updatedPrevRes->MatchLen
                , cmpData.at(updatedPrevRes->CmpDataOffset + updatedPrevRes->MatchLen)));

        if (it == prevIt->second.DiffRanges->end())
        {
            return std::nullopt;
        }

        return CompareWithCacheData(it->second.Offset, updatedPrevRes->CmpDataOffset, cmpData, updatedPrevRes->MatchLen);
    }

    void DiffCache::ConstructCache()
    {
        m_cache.clear();
        m_iteratorList.clear();
        const size_t dataSize = static_cast<uint32_t>(m_cacheData.size());

        for (size_t dataOffset = 0; dataOffset < dataSize; ++dataOffset)
        {
            auto it = m_cache.emplace(DiffCacheKey(0, m_cacheData.at(dataOffset)), DiffCacheValue(dataOffset));

            while (!it.second)
            {
                auto& parentKey = it.first->first;
                auto& parentValue = it.first->second;
                assert(dataOffset > parentValue.Offset);
                assert(m_cacheData.at(parentValue.Offset + parentKey.DiffOffset) == parentKey.DiffByte);

                size_t childCmpOffset = dataOffset + parentKey.DiffOffset;
                size_t parentCmpOffset = parentValue.Offset + parentKey.DiffOffset;

                for (
                    ; childCmpOffset < dataSize && m_cacheData.at(childCmpOffset) == m_cacheData.at(parentCmpOffset)
                    ; ++childCmpOffset, ++parentCmpOffset);

                // these data ranges must be equal:
                //
                // child data:            [dataOffset] ... (childCmpOffset)
                // parrent data: [parrentValue.Offset] ... (parentCmpOffset)
                assert(m_cacheData.substr(parentValue.Offset, parentCmpOffset - parentValue.Offset)
                    == m_cacheData.substr(dataOffset, childCmpOffset - dataOffset));

                if (dataSize == childCmpOffset)
                {
                    // this range is sub range of parent range
                    // so we don't need to save it
                    break;
                }

                // try to create node in parrent diff tree
                // with key - value of first different current str byte from parrent str 
                if (!parentValue.DiffRanges)
                {
                    parentValue.DiffRanges = std::make_unique<DiffCacheContainer>();
                }

                it = parentValue.DiffRanges->emplace(DiffCacheKey(childCmpOffset - dataOffset, m_cacheData.at(childCmpOffset))
                    , DiffCacheValue(dataOffset));
            }

            // save to iterator list parrent string
            m_iteratorList.push_back(it.first);
        }

        assert(m_cacheData.size() == m_iteratorList.size());
    }

    std::optional<CacheMatchResult> DiffCache::CompareWithCacheData(size_t cacheDataOffset,
        size_t cmpDataOffset,
        const Data& cmpData,
        size_t cachedMatchLen) const
    {
        CacheMatchResult res(cacheDataOffset, cmpDataOffset, cachedMatchLen);

        auto isFinishCompare = [cacheDataOffset, &cacheData = m_cacheData, cmpDataOffset, &cmpData, &res]()
        {
            return cacheDataOffset + res.MatchLen < cacheData.size()
                && cmpDataOffset + res.MatchLen < cmpData.size()
                && cacheData.at(cacheDataOffset + res.MatchLen) == cmpData.at(cmpDataOffset + res.MatchLen);
        };

        for (; isFinishCompare(); ++res.MatchLen);

        return res.MatchLen == 0 || res.MatchLen == cachedMatchLen ? std::nullopt : std::make_optional(res);
    }
}

