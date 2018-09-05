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

        auto diffStrings = std::ref(m_cache);
        auto it = diffStrings.get().find(NeedleKey(hs.at(hsOffset), 0));

        const size_t hsSize = hs.size();
        const size_t nlSize = m_needle.size(); 
        size_t currentNlOffset = 0;
        size_t currentHsOffset = hsOffset;

        while (it != diffStrings.get().end())
        {
            currentNlOffset = it->second.Offset + it->first.DiffOffset;

            for (; currentHsOffset < hsSize
                && currentNlOffset < nlSize
                && hs.at(currentHsOffset) == m_needle.at(currentNlOffset)
                ; ++currentNlOffset, ++currentHsOffset);

            if (handleResultCb)
            {
                // Send compare result to caller
                handleResultCb(Result(hsOffset, it->second.Offset, currentHsOffset - hsOffset));

                for (auto offset : it->second.SubStringOffsets)
                {
                    if (!handleResultCb(Result(hsOffset, offset, nlSize - offset)))
                    {
                        break;
                    }
                }
            }

            if (currentNlOffset == nlSize || currentHsOffset == hsSize)
            {
                // There are no more elements to compare
                return;
            }

            diffStrings = std::ref(it->second.DiffStrings);
            it = diffStrings.get().find(NeedleKey(hs.at(currentHsOffset), currentNlOffset - it->second.Offset));
        }
    }
#pragma warning (pop)
}