#pragma once
#include "DiffCacheTypes.h"
#include "Result.h"

namespace sf::lib::diff_cache
{
    using DiffCachePtr = std::unique_ptr<DiffCache>;
    DiffCachePtr Create(const std::string& cacheData);

    using Data = std::string;

    class DiffCacheWrapper
    {
    public:
        explicit DiffCacheWrapper(std::wstring dataPath);

        // Return first string, where match length >= 1
        std::optional<Result> Compare(uint32_t dataOffset, const Data& data) const;

        // Return compare result between string by diff offset and data
        std::optional<Result> CompareByDiffOffset(uint32_t cacheOffset
            , uint32_t diffOffset
            , uint32_t dataOffset
            , const Data& data) const;

        // Return offsets of strings which equal to parrent string but less than it
        std::optional<const OffsetList&> GetSubStrings(uint32_t offset) const;

    private:
        void FillCache();

    private:
        Data m_rawData;
        DiffCache m_cache;
        std::vector<ConstIterator> m_iteratorList;

    };
}