#include "stdafx.h"
#include "Matcher.h"

using Data = sf::lib::Data;
using namespace testing;

namespace
{
    struct MockNeedleCache : sf::lib::INeedleCache
    {
        MOCK_CONST_METHOD1(GetOffsetList, const sf::lib::OffsetList&(char));
        MOCK_CONST_METHOD0(GetNeedle, const Data&());
    };

    class TestMatcher : public Test
    {
    public:
        void SetParams(size_t threshold
            , size_t hsOffset
            , Data needle
            , Data haystack
            , sf::lib::OffsetList offsetList)
        {
            m_threshold = threshold;
            m_hsOffset = hsOffset;
            m_needle = std::move(needle);
            m_haystack = std::move(haystack);
            m_offsetList = std::move(offsetList);
        }

        void ResetMatcher()
        {
            m_matcher = std::make_unique<sf::lib::Matcher>(m_threshold, std::move(m_mock));
        }

    protected:
        size_t m_threshold = 0;
        size_t m_hsOffset = 0;
        Data m_needle;
        Data m_haystack;
        sf::lib::OffsetList m_offsetList;
        std::unique_ptr<MockNeedleCache> m_mock = std::make_unique<MockNeedleCache>();
        std::unique_ptr<sf::lib::Matcher> m_matcher;
    };
}

TEST_F(TestMatcher, ZeroThreshold)
{
    ASSERT_THROW(ResetMatcher(), std::runtime_error);
}

TEST_F(TestMatcher, MatchMiddle_EmptyOffsetList)
{
    SetParams(2, 1, "", "test", {});
    EXPECT_CALL(*m_mock, GetNeedle())
        .WillOnce(ReturnRef(m_needle));

    EXPECT_CALL(*m_mock, GetOffsetList(m_haystack.at(m_hsOffset)))
        .WillOnce(ReturnRef(m_offsetList));

    ASSERT_NO_THROW(ResetMatcher());
    ASSERT_EQ(0, m_matcher->Match(0, m_hsOffset, m_haystack));
    ASSERT_EQ(0, m_matcher->GetResults().size());
}

TEST_F(TestMatcher, MatchMiddle_LessThanThreshold)
{
    SetParams(3, 2, "test", "tester", { 2 });
    ASSERT_LT(m_needle.size() - m_offsetList.back(), m_threshold);

    EXPECT_CALL(*m_mock, GetNeedle())
        .WillOnce(ReturnRef(m_needle));

    EXPECT_CALL(*m_mock, GetOffsetList(m_haystack.at(m_hsOffset)))
        .WillOnce(ReturnRef(m_offsetList));

    ASSERT_NO_THROW(ResetMatcher());
    ASSERT_EQ(0, m_matcher->Match(0, m_hsOffset, m_haystack));
    ASSERT_EQ(0, m_matcher->GetResults().size());
}

TEST_F(TestMatcher, MatchMiddle_OneOffset)
{
    SetParams(2, 2, "needle", "hsneedle", { 0 });
    sf::lib::Result expectRes(m_hsOffset, m_offsetList[0], m_needle.size());

    EXPECT_CALL(*m_mock, GetNeedle())
        .WillOnce(ReturnRef(m_needle));

    EXPECT_CALL(*m_mock, GetOffsetList(m_haystack.at(m_hsOffset)))
        .WillOnce(ReturnRef(m_offsetList));

    ASSERT_NO_THROW(ResetMatcher());
    ASSERT_EQ(expectRes.MatchLen, m_matcher->Match(0, m_hsOffset, m_haystack));
    ASSERT_EQ(1, m_matcher->GetResults().size());

    auto& testRes = m_matcher->GetResults().back();
    ASSERT_EQ(expectRes.HsOffset, testRes.HsOffset);
    ASSERT_EQ(expectRes.NlOffset, testRes.NlOffset);
    ASSERT_EQ(expectRes.MatchLen, testRes.MatchLen);
}

TEST_F(TestMatcher, MatchMiddle_MaxResult)
{
    SetParams(2, 2, "neeneedleneed", "hsneedle", { 0, 3, 9 });
    sf::lib::Result expectRes(m_hsOffset, m_offsetList[1], sizeof("needle") - 1);

    EXPECT_CALL(*m_mock, GetNeedle())
        .WillOnce(ReturnRef(m_needle));

    EXPECT_CALL(*m_mock, GetOffsetList(m_haystack.at(m_hsOffset)))
        .WillOnce(ReturnRef(m_offsetList));

    std::unique_ptr<sf::lib::Matcher> matcher;
    ASSERT_NO_THROW(ResetMatcher());
    ASSERT_EQ(expectRes.MatchLen, m_matcher->Match(0, m_hsOffset, m_haystack));
    ASSERT_EQ(1, m_matcher->GetResults().size());

    auto& testRes = m_matcher->GetResults().back();
    ASSERT_EQ(expectRes.HsOffset, testRes.HsOffset);
    ASSERT_EQ(expectRes.NlOffset, testRes.NlOffset);
    ASSERT_EQ(expectRes.MatchLen, testRes.MatchLen);
}

TEST_F(TestMatcher, MatchMiddle_FixPrevResult)
{
    SetParams(3, 2, "needletest", "hsneedletest", { 0 });
    auto hs1 = m_haystack.substr(0, 8);
    auto hs2 = m_haystack.substr(8, 4);
    sf::lib::Result expectRes(m_hsOffset, m_offsetList[0], sizeof("needle") - 1);
    sf::lib::Result fixedExpectRes(m_hsOffset, m_offsetList[0], m_needle.size());

    EXPECT_CALL(*m_mock, GetNeedle())
        .Times(2)
        .WillRepeatedly(ReturnRef(m_needle));

    EXPECT_CALL(*m_mock, GetOffsetList(hs1.at(m_hsOffset)))
        .WillOnce(ReturnRef(m_offsetList));

    sf::lib::OffsetList offsetList2 = { 6 };
    EXPECT_CALL(*m_mock, GetOffsetList(hs2.at(0)))
        .WillOnce(ReturnRef(offsetList2));

    ASSERT_NO_THROW(ResetMatcher());

    // Call match function for first chunck of haystack
    ASSERT_EQ(expectRes.MatchLen, m_matcher->Match(0, m_hsOffset, hs1));
    ASSERT_EQ(1, m_matcher->GetResults().size());

    auto& testRes = m_matcher->GetResults().back();
    ASSERT_EQ(expectRes.HsOffset, testRes.HsOffset);
    ASSERT_EQ(expectRes.NlOffset, testRes.NlOffset);
    ASSERT_EQ(expectRes.MatchLen, testRes.MatchLen);

    // Call match function for second chunck of haystack
    // Matcher must fix previous result and return match length only of second part
    ASSERT_EQ(hs2.size(), m_matcher->Match(0, 0, hs2));
    ASSERT_EQ(1, m_matcher->GetResults().size());

    auto& fixedTestRes = m_matcher->GetResults().back();
    ASSERT_EQ(fixedExpectRes.HsOffset, fixedTestRes.HsOffset);
    ASSERT_EQ(fixedExpectRes.NlOffset, fixedTestRes.NlOffset);
    ASSERT_EQ(fixedExpectRes.MatchLen, fixedTestRes.MatchLen);
}

TEST_F(TestMatcher, MatchMiddle_FixHsOffset)
{
    SetParams(3, 2, "needletest", "hssomehstest", { 6 });
    auto hsPart = m_haystack.substr(6, 6);
    size_t hsIndex = 1;
    sf::lib::Result expectRes(m_hsOffset + (hsIndex * hsPart.size()), m_offsetList[0], sizeof("test") - 1);

    EXPECT_CALL(*m_mock, GetNeedle())
        .WillOnce(ReturnRef(m_needle));

    EXPECT_CALL(*m_mock, GetOffsetList(m_haystack.at(expectRes.HsOffset)))
        .WillOnce(ReturnRef(m_offsetList));

    ASSERT_NO_THROW(ResetMatcher());

    // Match function must fix hs offset 
    // and create result with value = m_hsOffset + (hsIndex * hsPart.size())
    ASSERT_EQ(expectRes.MatchLen, m_matcher->Match(hsIndex, m_hsOffset, hsPart));
    ASSERT_EQ(1, m_matcher->GetResults().size());

    auto& testRes = m_matcher->GetResults().back();
    ASSERT_EQ(expectRes.HsOffset, testRes.HsOffset);
    ASSERT_EQ(expectRes.NlOffset, testRes.NlOffset);
    ASSERT_EQ(expectRes.MatchLen, testRes.MatchLen);
}