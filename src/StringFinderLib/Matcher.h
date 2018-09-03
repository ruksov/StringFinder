#pragma once
#include "Result.h"
#include "IReader.h"
#include "INeedleCache.h"

namespace sf::lib
{
    using NeedleCachePtr = std::unique_ptr<INeedleCache>;

    class Matcher
    {
    public:
        Matcher(size_t threshold, NeedleCachePtr needleCache);

        size_t Match(size_t hsIndex, size_t hsOffset, const Data& hs);

    private:
        Result CompareData(size_t nlOffset, const Data& nl, size_t hsOffset, const Data& hs);
        
        Result MatchHaystack(size_t hsOffset, const Data& hs);      
        Result MatchHaystackBegin(const Data& hs);
        bool MatchHaystackEnd(size_t hsOffset, const Data& hs);

    private:
        ResultList m_results;
        std::map<size_t, Result> m_combineResults;
        const size_t m_threshold;
        const NeedleCachePtr m_needleCache;
    };
}