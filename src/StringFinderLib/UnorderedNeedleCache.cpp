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
            std::vector<size_t> matchLenList{ 0 };
            size_t subOffset = 0;
            size_t parrentOffset = 0; 
            size_t diffOffset = 0;

            for (auto c : needle)
            {
                assert(matchLenList.size() >= 1);
                auto it = cache.emplace(NeedleKey(c, 0), NeedleValue(i));
                auto matchLenIt = matchLenList.begin();

                while (!it.second)
                {
                    // save offset from which current node in diff tree different from root
                    diffOffset = it.first->first.DiffOffset;
                    
                    if (matchLenIt == matchLenList.end())
                    {
                        matchLenList.push_back(0);
                        matchLenIt = matchLenList.end() - 1;
                    }

                    if (*matchLenIt != 0)
                    {
                        // at this level of tree we already compare strings,
                        // so we get this matchLen - 1, becase it match length was in previous element
                        diffOffset += --*matchLenIt;
                    }

                    // get offsets for parrent and sub string 
                    // wich starts from different offset from previous comparation
                    subOffset = i + diffOffset;
                    parrentOffset = it.first->second.Offset + diffOffset;

                    for (; subOffset < nlSize && needle.at(parrentOffset) == needle.at(subOffset)
                        ; ++subOffset, ++parrentOffset)
                    {
                        ++*matchLenIt;
                    }

                    if (nlSize - subOffset == 0)
                    {
                        it.first->second.SubStringOffsets.push_back(i);
                        break;
                    }
                        
                    ++matchLenIt;
                    it = it.first->second.DiffStrings.emplace(
                            NeedleKey(needle.at(subOffset), subOffset - i),
                            NeedleValue(i));
                }

                //assert(matchLenIt == matchLenList.end() || matchLenIt == matchLenList.begin());
                if (matchLenList.size() != 1 && matchLenList.back() == 0)
                {
                    matchLenList.pop_back();
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

        auto isCacheEnd = [&res, nlSize]()
        {
            return res.It->second.Offset + res.MatchLen >= nlSize;
        };

        auto isDataEnd = [offset, &res, dataSize]()
        {
            return offset + res.MatchLen >= dataSize;
        };

        while (it != cache.get().cend())
        {
            res.It = it;
            for (
                ; !isCacheEnd() && !isDataEnd()
                && data.at(offset + res.MatchLen) == m_needle.at(it->second.Offset + res.MatchLen)
                ; ++res.MatchLen);

            if (isCacheEnd() || isDataEnd())
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