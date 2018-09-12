#include "stdafx.h"
#include "DiffCache.h"
#include "Exceptions.h"
#include "Log.h"

size_t g_cmpCount = 0;

namespace sf::lib
{
    DiffCache::DiffCache(Data cacheData)
        : m_cacheData(std::move(cacheData))
    {
        ConstructCache();
    }

    DiffCache::~DiffCache()
    {
        LOG_INFO("compare count - " << g_cmpCount);
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

    bool DiffCache::GetFirstResult(CacheMatchResult & inOutRes, const Data & cmpData)
    {
        std::optional<size_t> updatedCacheOffset;
        if (inOutRes.MatchLen != 0)
        {
            // find more suitable node for comparation
            auto it = FindHighestParent(inOutRes.MatchLen, m_iteratorList.at(inOutRes.CacheOffset));
            updatedCacheOffset = it->second.Offset;
        }
        else if (inOutRes.CmpDataOffset < cmpData.size())
        {
            auto it = m_cache.find(DiffCacheKey(0, cmpData.at(inOutRes.CmpDataOffset)));
            if (it != m_cache.end())
            {
                updatedCacheOffset = it->second.Offset;
            }
        }

        if (!updatedCacheOffset)
        {
            return false;
        }

        inOutRes.CacheOffset = updatedCacheOffset.value();
        size_t addMatchLen = CompareWithCacheData(inOutRes.CacheOffset
            , inOutRes.CmpDataOffset
            , cmpData
            , inOutRes.MatchLen);
        inOutRes.MatchLen += addMatchLen;
        return addMatchLen != 0;
    }

    bool DiffCache::GetNextResult(CacheMatchResult& inOutRes, const Data & cmpData)
    {
        auto prevIt = m_iteratorList.at(inOutRes.CacheOffset);
        if (!prevIt->second.DiffRanges                                          // range from prev result has not any diff sub ranges
            || inOutRes.CmpDataOffset + inOutRes.MatchLen >= cmpData.size())    // end of cmp data range
        {
            return false;
        }

        auto it = prevIt->second.DiffRanges->find(
            DiffCacheKey(inOutRes.MatchLen
                , cmpData.at(inOutRes.CmpDataOffset + inOutRes.MatchLen)));

        if (it == prevIt->second.DiffRanges->end())
        {
            return false;
        }

        inOutRes.CacheOffset = it->second.Offset;
        inOutRes.MatchLen += CompareWithCacheData(inOutRes.CacheOffset
            , inOutRes.CmpDataOffset
            , cmpData
            , inOutRes.MatchLen);

        return true;
    }

    void DiffCache::ConstructCache()
    {
        m_cache.clear();
        m_iteratorList.clear();
        const size_t dataSize = m_cacheData.size();

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
                    ; childCmpOffset < dataSize 
                    && m_cacheData.at(childCmpOffset) == m_cacheData.at(parentCmpOffset)
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
                    , DiffCacheValue(dataOffset, parentValue.Offset));
            }

            // save to iterator list parrent string
            m_iteratorList.push_back(it.first);
        }

        assert(m_cacheData.size() == m_iteratorList.size());
    }

    size_t DiffCache::CompareWithCacheData(size_t cacheDataOffset,
        size_t cmpDataOffset,
        const Data& cmpData,
        size_t cachedMatchLen) const
    {
        size_t matchLen = cachedMatchLen;

        for (; cacheDataOffset + matchLen < m_cacheData.size()
            && cmpDataOffset + matchLen < cmpData.size()
            && m_cacheData.at(cacheDataOffset + matchLen) == cmpData.at(cmpDataOffset + matchLen)
            ; ++matchLen, ++g_cmpCount);

        return matchLen - cachedMatchLen;
    }

    DiffCacheContainer::iterator& DiffCache::FindHighestParent(size_t matchLen, DiffCacheContainer::iterator & it)
    {
        auto parentIt = std::ref(it);
        while (parentIt.get()->second.Offset != parentIt.get()->second.ParentOffset
            && matchLen <= parentIt.get()->first.DiffOffset)
        {
            parentIt = std::ref(m_iteratorList.at(parentIt.get()->second.ParentOffset));
        }
        return parentIt;
    }
}

