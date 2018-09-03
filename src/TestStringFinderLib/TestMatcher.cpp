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
}

TEST(TestMatcher, ZeroThreshold)
{
    auto mock = std::make_unique<MockNeedleCache>();
    
    ASSERT_THROW(sf::lib::Matcher matcher(0, std::move(mock)), std::runtime_error);
}

TEST(TestMatcher, MatchMiddle_EmptyOffsetList)
{
    auto mock = std::make_unique<MockNeedleCache>();
    
    Data needle;
    Data haystack = "test";
    size_t threshold = 2;
    size_t hsOffset = 1;
    sf::lib::OffsetList emptyList;
    EXPECT_CALL(*mock, GetNeedle())
        .WillOnce(ReturnRef(needle));

    EXPECT_CALL(*mock, GetOffsetList(haystack.at(hsOffset)))
        .WillOnce(ReturnRef(emptyList));

    std::unique_ptr<sf::lib::Matcher> matcher;
    ASSERT_NO_THROW(matcher.reset(new sf::lib::Matcher(threshold, std::move(mock))));
    ASSERT_EQ(0, matcher->Match(0, hsOffset, haystack));
}

TEST(TestMatcher, MatchMiddle_LessThanThreshold)
{
    auto mock = std::make_unique<MockNeedleCache>();

    Data needle = "test";
    Data haystack = "tester";
    size_t threshold = 3;
    size_t hsOffset = 2;

    sf::lib::OffsetList offsetList = { 2 };
    ASSERT_LT(needle.size() - offsetList.back(), threshold);

    EXPECT_CALL(*mock, GetNeedle())
        .WillOnce(ReturnRef(needle));

    EXPECT_CALL(*mock, GetOffsetList(haystack.at(hsOffset)))
        .WillOnce(ReturnRef(offsetList));

    std::unique_ptr<sf::lib::Matcher> matcher;
    ASSERT_NO_THROW(matcher.reset(new sf::lib::Matcher(threshold, std::move(mock))));
    ASSERT_EQ(0, matcher->Match(0, hsOffset, haystack));
}

TEST(TestMatcher, MatchMiddle_OneOffset)
{
    auto mock = std::make_unique<MockNeedleCache>();

    Data needle = "needle";
    Data haystack = "myneedle";
    size_t threshold = 2;
    size_t hsOffset = 2;
    sf::lib::OffsetList offsetList = { 0 };
    EXPECT_CALL(*mock, GetNeedle())
        .WillOnce(ReturnRef(needle));

    EXPECT_CALL(*mock, GetOffsetList(haystack.at(hsOffset)))
        .WillOnce(ReturnRef(offsetList));

    std::unique_ptr<sf::lib::Matcher> matcher;
    ASSERT_NO_THROW(matcher.reset(new sf::lib::Matcher(threshold, std::move(mock))));
    ASSERT_EQ(needle.size(), matcher->Match(0, hsOffset, haystack));
}

TEST(TestMatcher, MatchMiddle_MaxResult)
{
    auto mock = std::make_unique<MockNeedleCache>();

    Data needle = "neeneedleneed";
    Data haystack = "myneedle";
    size_t threshold = 2;
    size_t hsOffset = 2;
    sf::lib::OffsetList offsetList = { 0, 3, 9 };
    EXPECT_CALL(*mock, GetNeedle())
        .WillOnce(ReturnRef(needle));

    EXPECT_CALL(*mock, GetOffsetList(haystack.at(hsOffset)))
        .WillOnce(ReturnRef(offsetList));

    std::unique_ptr<sf::lib::Matcher> matcher;
    ASSERT_NO_THROW(matcher.reset(new sf::lib::Matcher(threshold, std::move(mock))));
    ASSERT_EQ(sizeof("needle") - 1, matcher->Match(0, hsOffset, haystack));
}