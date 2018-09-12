#include "stdafx.h"
#include "Diffcache.h"

using namespace sf::lib;
using namespace testing;

TEST(TestDiffCache, EmptyData)
{
    std::string data = "";
    auto cache = diff_cache::Create(data);
    ASSERT_TRUE(cache->empty());
}

TEST(TestDiffCache, OneByteData)
{
    std::string data = "a";
    auto cache = diff_cache::Create(data);

    ASSERT_EQ(1, cache->size());
    auto node = cache->begin();

    ASSERT_EQ(data.at(0), node->first.DiffChar);
    ASSERT_EQ(0, node->first.DiffOffset);

    ASSERT_EQ(0, node->second.Offset);
    ASSERT_FALSE(node->second.NextDataByte);
    ASSERT_FALSE(node->second.DiffStrings);
    ASSERT_FALSE(node->second.SubStrings);
}

TEST(TestDiffCache, TwoEqualBytes)
{
    std::string data = "aa";
    auto cache = diff_cache::Create(data);

    ASSERT_EQ(1, cache->size());
    auto node = cache->begin();

    ASSERT_EQ(data.at(0), node->first.DiffChar);
    ASSERT_EQ(0, node->first.DiffOffset);

    ASSERT_EQ(0, node->second.Offset);
    ASSERT_FALSE(node->second.NextDataByte);
    ASSERT_FALSE(node->second.DiffStrings);

    ASSERT_EQ(1, node->second.SubStrings->size());
    
    const size_t subStringOffset = 1;
    ASSERT_EQ(subStringOffset, node->second.SubStrings->at(0));
}

TEST(TestDiffCache, MultiEqualBytes)
{
    std::string data = "aaaaaaaa";
    auto cache = diff_cache::Create(data);

    ASSERT_EQ(1, cache->size());
    auto node = cache->begin();

    ASSERT_EQ(data.at(0), node->first.DiffChar);
    ASSERT_EQ(0, node->first.DiffOffset);

    ASSERT_EQ(0, node->second.Offset);
    ASSERT_FALSE(node->second.NextDataByte);
    ASSERT_FALSE(node->second.DiffStrings);

    ASSERT_EQ(data.size() - 1, node->second.SubStrings->size());

    size_t testOffset = 0;
    for (auto offset : *node->second.SubStrings)
    {
        ASSERT_EQ(++testOffset, offset);
    }
}

TEST(TestDiffCache, TwoLevelDiffTree_CheckFirstLevel)
{
    std::string data = "aaaaBaaa";
    auto cache = diff_cache::Create(data);

    ASSERT_EQ(2, cache->size());

    auto aTree = cache->find(diff_cache::Key('a', 0));
    ASSERT_NE(cache->end(), aTree);

    auto BTree = cache->find(diff_cache::Key('B', 0));
    ASSERT_NE(cache->end(), BTree);

    // check B-tree
    ASSERT_EQ('B', BTree->first.DiffChar);
    ASSERT_EQ(0, BTree->first.DiffOffset);

    ASSERT_EQ(data.find('B'), BTree->second.Offset);
    ASSERT_FALSE(BTree->second.NextDataByte);
    ASSERT_FALSE(BTree->second.DiffStrings);
    ASSERT_FALSE(BTree->second.SubStrings);

    // check a-tree
    ASSERT_EQ('a', aTree->first.DiffChar);
    ASSERT_EQ(0, aTree->first.DiffOffset);

    ASSERT_EQ(0, aTree->second.Offset);

    // check a-tree sub strings
    ASSERT_EQ(3, aTree->second.SubStrings->size());

    size_t testSubOffset = 5;
    for (auto offset : *aTree->second.SubStrings)
    {
        ASSERT_EQ(testSubOffset++, offset);
    }
}

TEST(TestDiffCache, TwoLevelDiffTree_CheckSecondLevel)
{
    std::string data = "aaaaBaaa";
    auto cache = diff_cache::Create(data);

    ASSERT_EQ(2, cache->size());

    auto aTree = cache->find(diff_cache::Key('a', 0));
    ASSERT_NE(cache->end(), aTree);

    auto BTree = cache->find(diff_cache::Key('B', 0));
    ASSERT_NE(cache->end(), BTree);

    // check a-tree diff str tree
    ASSERT_EQ(3, aTree->second.DiffStrings->size());

    uint32_t diffOffset = 3;
    size_t diffStrStartOffset = 1;
    while (diffOffset != 0)
    {
        auto diffStr = aTree->second.DiffStrings->find(diff_cache::Key('B', diffOffset));
        ASSERT_NE(aTree->second.DiffStrings->end(), diffStr);

        ASSERT_EQ(diffStrStartOffset, diffStr->second.Offset);
        ASSERT_FALSE(diffStr->second.DiffStrings);
        ASSERT_FALSE(diffStr->second.SubStrings);

        size_t i = diffStr->second.Offset;
        for (; i < diffStr->second.Offset + diffStr->first.DiffOffset; ++i)
        {
            ASSERT_EQ('a', data.at(i));
        }

        ASSERT_EQ('B', data.at(i));

        --diffOffset;
        ++diffStrStartOffset;
    }
}

TEST(TestDiffCache, TwoLevelDiffTree_CheckNextDataByte)
{
    std::string data = "aaaaBaaa";
    auto cache = diff_cache::Create(data);

    ASSERT_EQ(2, cache->size());

    auto it = cache->find(diff_cache::Key('a', 0));
    ASSERT_NE(cache->end(), it);

    size_t testIndex = 0;
    while (it->second.NextDataByte)
    {
        ASSERT_EQ(testIndex, it->second.Offset);

        it = *it->second.NextDataByte;
        ++testIndex;
    }
}