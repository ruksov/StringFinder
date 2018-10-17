#include "stdafx.h"
#include "DiffCacheWrapper.h"
#include "Exceptions.h"
#include "Log.h"

namespace sf::lib
{
    constexpr size_t kMaxOptimalDataSize = 1'000'000;

    DiffCacheWrapper::DiffCacheWrapper(std::string dataPath)
    {
        {
            std::ifstream file;
            file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            file.open(dataPath, std::ios::in | std::ios::binary);
            m_cacheData.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        }

        if (m_cacheData.size() > kMaxOptimalDataSize)
        {
            LOG_INFO("Size of data for diff cahce is bigger than max optimal size\n"
                << "\t Actual cache data size = " << m_cacheData.size() << '\n'
                << "\t Max optimal data size = " << kMaxOptimalDataSize << '\n');
        }

        m_cache = diff_cache::Create(m_cacheData, m_iteratorList);
    }

    std::optional<Result> DiffCacheWrapper::CompareFirst(size_t dataOffset, const Data & data) const
    {
        THROW_IF(dataOffset >= data.size(), "Wrong imput parameter for data [dataOffset=" << dataOffset << "]");

        const auto subStrIt = m_cache->find(diff_cache::Key(0, data.at(dataOffset)));
        if (subStrIt == m_cache->end())
        {
            return std::nullopt;
        }

        const auto matchLen = CompareData(subStrIt->second.Offset, m_cacheData, dataOffset, data);
        if (matchLen == 0)
        {
            return std::nullopt;
        }

        return Result(dataOffset, subStrIt->second.Offset, matchLen);
    }

    std::optional<Result> DiffCacheWrapper::CompareNext(size_t cacheOffset,
        size_t diffOffset,
        size_t dataOffset,
        const Data & data) const
    {
        THROW_IF(dataOffset >= data.size(), "Wrong imput parameter for data [dataOffset=" << dataOffset << "]");

        if (dataOffset + diffOffset >= m_cacheData.size()
            || cacheOffset + diffOffset >= m_cacheData.size())
        {
            return std::nullopt;
        }

        const auto it = m_iteratorList.at(cacheOffset);

        // search string by diffOffset only if string[cacheOffset:] is not sub string
        if (it->second.Offset != cacheOffset)
        {
            // cache offset must be in sub string container in 'it' object
            assert(it->second.SubStrings != nullptr);
            assert(std::is_sorted(it->second.SubStrings->begin(), it->second.SubStrings->end()));
            assert(std::binary_search(it->second.SubStrings->begin(), it->second.SubStrings->end(), cacheOffset));
            return std::nullopt;
        }

        if (!it->second.DiffStrings)
        {
            return std::nullopt;
        }

        const auto subIt = it->second.DiffStrings->find(
            diff_cache::Key(static_cast<uint32_t>(diffOffset), data.at(dataOffset + diffOffset))
        );
        if (subIt == it->second.DiffStrings->end())
        {
            return std::nullopt;
        }

        const auto matchLen = CompareData(static_cast<size_t>(subIt->second.Offset) + diffOffset
            , m_cacheData
            , dataOffset + diffOffset
            , data);
        if (matchLen == 0)
        {
            return std::nullopt;
        }

        return Result(dataOffset, subIt->second.Offset, diffOffset + matchLen);
    }

    const diff_cache::OffsetList& DiffCacheWrapper::GetSubStrings(size_t offset) const
    {
        THROW_IF(offset >= m_iteratorList.size(), "Wrong input parameter for cache data [offset=" << offset << "]");

        auto it = m_iteratorList.at(offset);
        if (it->second.Offset == offset
            && it->second.SubStrings)
        {
            // we found offset in needle that has substrings 
            return *it->second.SubStrings;
        }

        // passed offset must be in found iterators sub strings container
        assert(it->second.SubStrings != nullptr);
        assert(std::is_sorted(it->second.SubStrings->begin(), it->second.SubStrings->end()));
        assert(std::binary_search(it->second.SubStrings->begin(), it->second.SubStrings->end(), offset));

        return m_emptySubStrings;
    }

    const size_t DiffCacheWrapper::GetCacheDataSize() const noexcept
    {
        return m_cacheData.size();
    }

    size_t DiffCacheWrapper::CompareData(size_t lOffset, const Data & lData, size_t rOffset, const Data & rData) const
    {
        size_t matchLen = 0;

        for (; lOffset + matchLen < lData.size() && rOffset + matchLen < rData.size()
            && lData.at(lOffset + matchLen) == rData.at(rOffset + matchLen);
            ++matchLen);

        return matchLen;
    }
}