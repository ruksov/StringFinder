#pragma once
#include "Result.h"
#include "DiffCacheWrapper.h"

namespace sf::lib
{
    using LogResultFn = std::function<void(Result)>;
    using DiffCacheRef = std::reference_wrapper<diff_cache::DiffCache>;

    class Matcher
    {
    public:
        Matcher(size_t threshold, std::string needlePath, LogResultFn logResultFn);
        ~Matcher();
        size_t Match(size_t hsDataIndex, size_t hsDataOffset, const Data& hsData);

    private:
        size_t CompareWithHaystack(size_t nlOffset, size_t hsOffset, const Data & hsData) const;
        std::optional<Result> GetResultFromPrevChunck(const Data& hsData, size_t& matchLenInCurrentChunck);

        std::optional<Result> FindMaxResult(size_t hsOffset, const Data& hsData) const;
        std::optional<Result> FindMaxResult(DiffCacheRef cache, 
            diff_cache::Iterator it, 
            size_t hsOffset, 
            const Data& hsData) const;

        std::optional<Result> FindMaxResult(Result res, size_t hsOffset, const Data& hsData) const;
        Result FindMaxResult_BasedOnPrevResult(size_t hsOfsset, const Data& hsData) const;

    private:
        size_t m_threshold;
        LogResultFn m_logResultFn;
        std::optional<Result> m_prevRes;
        Data m_needleData;
        diff_cache::DiffCachePtr m_needleCache;
        diff_cache::IteratorList m_needleIteratorList;
    };
}
