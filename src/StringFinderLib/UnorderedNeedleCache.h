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
        Data m_needle;
        NeedleStrHashTable m_cache;
        std::future<void> m_waitInit;
    };
}