#pragma once
#include "DiffCache.h"
#include "Result.h"

namespace sf::lib
{
    using Data = diff_cache::Data;

    class DiffCacheWrapper
    {
    public:
        explicit DiffCacheWrapper(std::string dataPath);

        // Find first string, which matches data with match lenght at least one byte 
        std::optional<Result> CompareFirst(uint32_t dataOffset, const Data& data) const;

        // Return compare result between string from diff tree and data
        std::optional<Result> CompareNext(uint32_t cacheOffset,
            uint32_t diffOffset,
            uint32_t dataOffset,
            const Data& data) const;

        // Return offsets of strings which equal to parrent string but less than it
        const diff_cache::OffsetList& GetSubStrings(uint32_t offset) const;

        const size_t GetCacheDataSize() const noexcept;

    private:
        size_t CompareData(size_t lOffset, const Data & lData, size_t rOffset, const Data & rData) const;

    private:
        Data m_cacheData;
        diff_cache::DiffCachePtr m_cache;
        std::vector<diff_cache::ConstIterator> m_iteratorList;
        const diff_cache::OffsetList m_emptyOffsetList;
    };
}

