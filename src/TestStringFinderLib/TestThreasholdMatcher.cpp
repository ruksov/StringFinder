#include "stdafx.h"
#include "MatcherFactory.h"
#include "CacheFactory.h"

using namespace testing;
using namespace sf::lib;

namespace
{
    struct TestThreasholdMatcher : public Test
    {
    public:
        void CreateMatcher()
        {
            m_matcher = MatcherFactory(MatcherType::ThreasholdMatcher
                , m_threashold
                , CacheFactory(CacheType::DiffCache, std::move(m_cacheData)));

            auto handler = std::make_shared<ResultHandlerFn>();
            *handler = [this](const MatchResult& res){ ResultHandler(res); };
            m_matcher->AddResultHandler(handler);
        }

        void ResultHandler(const MatchResult& res)
        {
            m_lastRes = res;
        }

        std::string GetCacheDataSubStr(const CacheMatchResult& res)
        {
            return m_cacheData.substr(res.CacheOffset, res.MatchLen);
        }

        std::string GetCmpDataSubStr(const CacheMatchResult& res)
        {
            return m_cmpData.substr(res.CmpDataOffset, res.MatchLen);
        }

        void CheckLastResult(const MatchResult& expectRes)
        {
            ASSERT_TRUE(m_lastRes);
            ASSERT_EQ(expectRes.HsDataIndex, m_lastRes->HsDataIndex);
            ASSERT_EQ(expectRes.HsDataOffset, m_lastRes->HsDataOffset);
            ASSERT_EQ(expectRes.NlOffset, m_lastRes->NlOffset);
            ASSERT_EQ(expectRes.MatchLen, m_lastRes->MatchLen);
        }

    protected:
        size_t m_threashold = 0;
        Data m_cmpData;
        Data m_cacheData;
        MatcherPtr m_matcher;

        std::optional<MatchResult> m_lastRes;
    };
}

TEST_F(TestThreasholdMatcher, DefaultCtor)
{
    ASSERT_NO_THROW(CreateMatcher());
}

TEST_F(TestThreasholdMatcher, MiddleChunckMatch)
{
    m_cacheData = "aaabbbbaaa";
    m_threashold = 3;
    ASSERT_NO_THROW(CreateMatcher());

    m_cmpData = "ccbbbbbcc";
    MatchResult expectRes(0);
    expectRes.HsDataOffset = 2;
    expectRes.NlOffset = 3;
    expectRes.MatchLen = 4;

    ASSERT_EQ(4, m_matcher->Match(expectRes.HsDataOffset, expectRes.HsDataIndex, m_cmpData));
    CheckLastResult(expectRes);
}

TEST_F(TestThreasholdMatcher, EndChunckMatch)
{
    m_cacheData = "aabbbbaa";
    m_threashold = 3;
    ASSERT_NO_THROW(CreateMatcher());

    m_cmpData = "ccccbbbb";
    MatchResult expectRes(0);
    expectRes.HsDataOffset = 4;
    expectRes.NlOffset = 2;
    expectRes.MatchLen = 4;

    ASSERT_EQ(4, m_matcher->Match(expectRes.HsDataOffset, expectRes.HsDataIndex, m_cmpData));
    ASSERT_FALSE(m_lastRes);

    ASSERT_EQ(0, m_matcher->Match(0, 1, Data()));
    CheckLastResult(expectRes);
}

TEST_F(TestThreasholdMatcher, EndChunckMatch_LessThanThreashold)
{
    m_cacheData = "aabbabbbbbbaa";
    m_threashold = 3;
    ASSERT_NO_THROW(CreateMatcher());

    MatchResult expectRes(0);
    expectRes.HsDataOffset = 11;
    expectRes.NlOffset = 5;
    expectRes.MatchLen = 5;

    m_cmpData = "cccccccccccbb";
    ASSERT_EQ(2, m_matcher->Match(11, 0, m_cmpData));
    ASSERT_FALSE(m_lastRes);

    m_cmpData = "bbbcccccccccc";
    ASSERT_EQ(3, m_matcher->Match(0, 1, m_cmpData));
    CheckLastResult(expectRes);
}