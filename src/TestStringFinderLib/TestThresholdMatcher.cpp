#include "stdafx.h"
#include "MatcherFactory.h"
#include "CacheFactory.h"

using namespace testing;
using namespace sf::lib;

namespace
{
    struct TestThresholdMatcher : public Test
    {
    public:
        void CreateMatcher()
        {
            m_matcher = MatcherFactory(MatcherType::ThresholdMatcher
                , m_threshold
                , CacheFactory(CacheType::DiffCache, m_needle));

            auto handler = std::make_shared<ResultHandlerFn>();
            *handler = [this](const MatchResult& res){ ResultHandler(res); };
            m_matcher->AddResultHandler(handler);
        }

        void ResultHandler(const MatchResult& res)
        {
            m_lastRes = res;
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
        size_t m_threshold = 0;
        Data m_haystack;
        Data m_needle;
        MatcherPtr m_matcher;

        std::optional<MatchResult> m_lastRes;
    };
}

TEST_F(TestThresholdMatcher, Ctor_Threshold_GT_NeedleSize)
{
    m_needle = "nl";
    m_threshold = 42;
    ASSERT_GT(m_threshold, m_needle.size());

    ASSERT_ANY_THROW(CreateMatcher());
}

TEST_F(TestThresholdMatcher, Ctor_NullptrCache)
{
    auto createMatcher = []()
    {
        auto matcher = MatcherFactory(MatcherType::ThresholdMatcher, 10, nullptr);
    };

    ASSERT_ANY_THROW(createMatcher());
}

TEST_F(TestThresholdMatcher, Match_NullMatch)
{
    m_needle = "needle";
    m_threshold = 3;
    m_haystack = "haystack";

    ASSERT_NO_THROW(CreateMatcher());

    ASSERT_EQ(0, m_matcher->Match(0, 0, m_haystack));
    ASSERT_FALSE(m_lastRes);
}

TEST_F(TestThresholdMatcher, Match_Midlle)
{
    m_needle = "aaabbbbaaa";
    m_threshold = 3;
    ASSERT_NO_THROW(CreateMatcher());

    m_haystack = "ccbbbbbcc";
    MatchResult expectRes(0);
    expectRes.HsDataOffset = 2;
    expectRes.NlOffset = 3;
    expectRes.MatchLen = 4;

    ASSERT_EQ(4, m_matcher->Match(expectRes.HsDataOffset, expectRes.HsDataIndex, m_haystack));
    CheckLastResult(expectRes);
}

TEST_F(TestThresholdMatcher, Match_Bound_LT_Threshold)
{
    m_needle = "bbbb";
    m_threshold = 3;
    ASSERT_NO_THROW(CreateMatcher());

    m_haystack = "ccbbbbcc";
    auto hsData1 = m_haystack.substr(0, 4);
    auto hsData2 = m_haystack.substr(4, 4);
    MatchResult expectRes(0);
    expectRes.HsDataOffset = 2;
    expectRes.NlOffset = 0;
    expectRes.MatchLen = 4;

    ASSERT_EQ(2, m_matcher->Match(2, 0, hsData1));
    ASSERT_FALSE(m_lastRes);

    ASSERT_EQ(2, m_matcher->Match(0, 1, hsData2));
    CheckLastResult(expectRes);
}

TEST_F(TestThresholdMatcher, Match_Bound_GE_Threshold)
{
    m_needle = "bbbb";
    m_threshold = 3;
    ASSERT_NO_THROW(CreateMatcher());

    m_haystack = "cbbbbccc";
    auto hsData1 = m_haystack.substr(0, 4);
    auto hsData2 = m_haystack.substr(4, 4);
    MatchResult expectRes(0);
    expectRes.HsDataOffset = 1;
    expectRes.NlOffset = 0;
    expectRes.MatchLen = 4;

    ASSERT_EQ(3, m_matcher->Match(1, 0, hsData1));
    ASSERT_FALSE(m_lastRes);

    ASSERT_EQ(1, m_matcher->Match(0, 1, hsData2));
    CheckLastResult(expectRes);
}

TEST_F(TestThresholdMatcher, Match_MaxResult)
{
    m_needle = "baaabaaaaabaa";
    m_threshold = 3;
    ASSERT_NO_THROW(CreateMatcher());

    m_haystack = "baaaaaaa";
    MatchResult expectRes(0);
    expectRes.HsDataOffset = 0;
    expectRes.NlOffset = 4;
    expectRes.MatchLen = 6;

    ASSERT_EQ(6, m_matcher->Match(0, 0, m_haystack));
    CheckLastResult(expectRes);
}

TEST_F(TestThresholdMatcher, Match_Bound_KnownBug_FailedTest)
{
    m_needle = "baacadcaaaaaadf";
    m_threshold = 8;

    m_haystack = "aaaaaaaaaabaacaaaaaaddddddddd";
    auto hsData1 = m_haystack.substr(0, m_needle.size());
    auto hsData2 = m_haystack.substr(m_needle.size(), m_needle.size());

    ASSERT_NO_THROW(CreateMatcher());

    // expect match range: caaaaaad
    // needle range: baacadCAAAAAADf
    // haystack range: aaaaaaaaaabaaCA|AAAAADdddddddd
    //
    // Bug description:
    // Matcher first match baaca in needle data and save it 
    // to continue matching with next haystack data chunk,
    // because we went to end first haystack data chunk.
    // Due to this saving we miss real match result, 
    // which equal threshold: caaaaaad 
    MatchResult expectRes(0);
    expectRes.HsDataOffset = m_haystack.find('c');
    expectRes.NlOffset = 7;     // offset of second 'c' in needle data
    expectRes.MatchLen = 8;
    
    // start possiotion in first haystack data chunk in upper case:
    // aaaaaaaaaaBaaca

    size_t i = m_haystack.find('b');
    while (i < hsData1.size())
    {
        auto matchLen = m_matcher->Match(i, 0, hsData1);
        i += matchLen == 0 ? 1 : matchLen;
    }

    i = 0;
    while (i < hsData2.size())
    {
        auto matchLen = m_matcher->Match(i, 0, hsData1);
        i += matchLen == 0 ? 1 : matchLen;
    }

    CheckLastResult(expectRes);
}