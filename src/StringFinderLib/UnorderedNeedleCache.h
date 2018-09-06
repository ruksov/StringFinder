#pragma once
#include "Result.h"
#include "UnorderedNeedleDataTypes.h"

namespace sf::lib
{
    using Data = std::string;
    using HadleResultCb = std::function<bool(Result)>;

    class UnorderedNeedleCache
    {
    public:
        UnorderedNeedleCache(std::wstring needlePath);

        void CompareData(size_t hsOffset, const Data& hs, HadleResultCb handleResultCb = nullptr);

    private:
        using ConstCacheValue = NeedleStrHashTable::const_iterator;
        struct CompareResult
        {
            ConstCacheValue It;
            size_t MatchLen = 0;
        }; 
        
    private:
        std::optional<CompareResult> FindLastValue(size_t offset, const Data& data);

    private:
        Data m_needle;
        NeedleStrHashTable m_cache;
        std::future<void> m_waitInit;
    };
}