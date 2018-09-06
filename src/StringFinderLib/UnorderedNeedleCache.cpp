#include "stdafx.h"
#include "UnorderedNeedleCache.h"

namespace sf::lib
{
// This warning is bug
#pragma warning (push)
#pragma warning (disable : 26489)
    UnorderedNeedleCache::UnorderedNeedleCache(std::wstring needlePath)
    {
        m_waitInit = std::async(std::launch::async, 
            [&cache = m_cache, &needle = m_needle](std::wstring filePath)
        {
            {
                std::ifstream file;
                file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
                file.open(filePath, std::ios::in | std::ios::binary);
                needle.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            }
            
            size_t i = 0;
            const size_t nlSize = needle.size();
            size_t subOffset = 0;
            size_t parrentOffset = 0; 
            for (auto c : needle)
            {
                auto it = cache.emplace(NeedleKey(c, 0), NeedleValue(i));

                while (!it.second)
                {
                    subOffset = i + it.first->first.DiffOffset;
                    parrentOffset = it.first->second.Offset + it.first->first.DiffOffset;

                    for (; subOffset < nlSize && needle.at(parrentOffset) == needle.at(subOffset)
                        ; ++subOffset, ++parrentOffset);

                    if (nlSize - subOffset == 0)
                    {
                        it.first->second.SubStringOffsets.push_back(i);
                        break;
                    }

                    it = it.first->second.DiffStrings.emplace(
                        NeedleKey(needle.at(subOffset), subOffset - i),
                        NeedleValue(i));
                }

                ++i;
            }
        }, std::move(needlePath));
    }

    void UnorderedNeedleCache::CompareData(size_t hsOffset, const Data & hs, HadleResultCb handleResultCb)
    {
        if (m_waitInit.valid())
        {
            m_waitInit.get();
        }

        auto res = FindLastValue(hsOffset, hs);
        if (!res)
        {
            return;
        }

        if (handleResultCb)
        {
            if (handleResultCb(Result(hsOffset, res.value().It->second.Offset, res.value().MatchLen)))
            {
                for (auto subStrOffset : res.value().It->second.SubStringOffsets)
                {
                    handleResultCb(Result(hsOffset, subStrOffset, res.value().MatchLen));
                }
            }
        }
    }

    std::optional<UnorderedNeedleCache::CompareResult> UnorderedNeedleCache::FindLastValue(size_t offset, const Data & data)
    {
        CompareResult res;
        auto cache = std::ref(m_cache);
        const size_t nlSize = m_needle.size();
        const size_t dataSize = data.size();
        auto it = m_cache.find(NeedleKey(data.at(offset), 0));

        while (it != cache.get().cend())
        {
            res.It = it;
            bool isCacheEnd = it->second.Offset + res.MatchLen < nlSize;
            bool isDataEnd = offset + res.MatchLen < dataSize;

            for (
                ; isCacheEnd && isDataEnd 
                && data.at(offset + res.MatchLen) == m_needle.at(it->second.Offset + res.MatchLen)
                ; ++res.MatchLen)
            {
                isCacheEnd = it->second.Offset + res.MatchLen < nlSize;
                isDataEnd = offset + res.MatchLen < dataSize;
            }

            if (isCacheEnd || isDataEnd)
            {
                break;
            }

            cache = std::ref(it->second.DiffStrings);
            it = cache.get().find(NeedleKey(data.at(offset + res.MatchLen), res.MatchLen));
        }

        return res.MatchLen != 0 ? std::make_optional(res) : std::nullopt;
    }
#pragma warning (pop)
}