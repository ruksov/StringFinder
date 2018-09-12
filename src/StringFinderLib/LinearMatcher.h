#pragma once
#include "Result.h"
#include "DiffCache.h"

namespace sf::lib
{
    using Data = std::string;

    class LinearMatcher
    {
    public:
        LinearMatcher(size_t threshold, std::wstring filePath);

        size_t Match(size_t hsIndex, size_t hsOffset, const Data& hs);

    private:
        size_t CompareData(size_t nlOffset, size_t hsOffset, const Data& hs);

        std::optional<Result> MatchHaystackBegin(const Data& hs);
        std::optional<Result> MatchHaystackMiddle(size_t hsOffset, const Data& hs);
        bool MatchHaystackEnd(size_t hsOffset, const Data& hs);

        void PushToResults(size_t hsIndex, size_t hsSize, Result& res, bool isCombineResult);

    private:
        size_t m_threshold;
        Result m_cacheRes;
        std::ofstream m_resultLog;
        std::map<size_t, Result> m_combineResults;
        Data m_needle;
        diff_cache::DiffCachePtr m_cache;

    };
}


