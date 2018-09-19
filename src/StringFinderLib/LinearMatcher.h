#pragma once
#include "Result.h"
#include "DiffCacheWrapper.h"

namespace sf::lib
{
    class LinearMatcher
    {
    public:
        LinearMatcher(size_t threshold, std::wstring filePath);

        size_t Match(size_t hsIndex, size_t hsOffset, const Data& hs);

    private:
        std::optional<Result> MatchHaystackBegin(const Data& hs);
        std::optional<Result> MatchHaystackMiddle(size_t hsOffset, const Data& hs);
        bool MatchHaystackEnd(size_t hsOffset, const Data& hs);

        std::optional<Result> FindResFromPrevHsData(uint32_t nlOffset);

        void PushToResults(size_t hsIndex, size_t hsSize, Result& res, bool isCombineResult);

    private:
        size_t m_threshold;
        Result m_cacheRes;
        Result m_optimizedRes;
        std::ofstream m_resultLog;
        std::map<size_t, Result> m_combineResults;
        std::unique_ptr<DiffCacheWrapper> m_cache;
    };
}


