#include "stdafx.h"
#include "SortedNeedleCache.h"

TEST(TestSortedNeedleCache, GetNeedle)
{
    const char testNdl[] = "test Needle";
    sf::lib::SortedNeedleCache ndlCache(testNdl);

    auto& ndl = ndlCache.GetNeedle();
    ASSERT_STREQ(testNdl, ndl.c_str());
}

TEST(TestSortedNeedleCache, GetIndexList_EmptyNeedle)
{
    const char testNdl[] = "";
    sf::lib::SortedNeedleCache ndlCache(testNdl);

    const auto charMin = std::numeric_limits<char>::min();
    char c = std::numeric_limits<char>::min();

    do
    {
        auto& indxList = ndlCache.GetIndexList(c);
        ASSERT_EQ(0, indxList.size());
        ++c;
    } while (c != charMin);
}

TEST(TestSortedNeedleCache, GetIndexList_Normal)
{
    const char testNdl[] = "testtest";
    sf::lib::SortedNeedleCache ndlCache(testNdl);

    // get index list for all needle character
    auto& tIndxList = ndlCache.GetIndexList('t');
    auto& eIndxList = ndlCache.GetIndexList('e');
    auto& sIndxList = ndlCache.GetIndexList('s');

    auto checkList = [&testNdl](char c, size_t charCount, auto& indxList)
    {
        ASSERT_EQ(charCount, indxList.size());
        ASSERT_TRUE(std::is_sorted(indxList.begin(), indxList.end()));
        for (auto i : indxList)
        {
            ASSERT_EQ(c, testNdl[i]);
        }
    };

    checkList('t', 4, tIndxList);
    checkList('e', 2, eIndxList);
    checkList('s', 2, sIndxList);

    const auto charMin = std::numeric_limits<char>::min();
    char c = std::numeric_limits<char>::min();

    do
    {
        if (c != 't' && c != 'e' && c != 's')
        {
            // index lists for all other chars must be empty
            auto& indxList = ndlCache.GetIndexList(c);
            ASSERT_EQ(0, indxList.size());
        }
        ++c;
    } while (c != charMin);
}