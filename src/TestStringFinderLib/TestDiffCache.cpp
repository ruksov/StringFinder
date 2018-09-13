#include "stdafx.h"
#include "Diffcache.h"

using namespace sf::lib;
using namespace testing;

TEST(TestDiffCache, EmptyData)
{
    std::string data = "";
    diff_cache::IteratorList itList;
    auto cache = diff_cache::Create(data, itList);
    ASSERT_TRUE(cache->empty());
}

TEST(TestDiffCache, OneByteData)
{
    std::string data = "a";
    diff_cache::IteratorList itList;
    auto cache = diff_cache::Create(data, itList);

    ASSERT_EQ(1, cache->size());
    auto node = cache->begin();

    ASSERT_EQ(data.at(0), node->first.Info.Byte);
    ASSERT_EQ(0, node->first.Info.Offset);

    ASSERT_EQ(0, node->second.Offset);
    ASSERT_FALSE(node->second.DiffStrings);
    ASSERT_FALSE(node->second.SubStrings);
}

TEST(TestDiffCache, TwoEqualBytes)
{
    std::string data = "aa";
    diff_cache::IteratorList itList;
    auto cache = diff_cache::Create(data, itList);

    ASSERT_EQ(1, cache->size());
    auto node = cache->begin();

    ASSERT_EQ(data.at(0), node->first.Info.Byte);
    ASSERT_EQ(0, node->first.Info.Offset);

    ASSERT_EQ(0, node->second.Offset);
    ASSERT_FALSE(node->second.DiffStrings);

    ASSERT_EQ(1, node->second.SubStrings->size());
    
    const size_t subStringOffset = 1;
    ASSERT_EQ(subStringOffset, node->second.SubStrings->at(0));
}

TEST(TestDiffCache, MultiEqualBytes)
{
    std::string data = "aaaaaaaa";
    diff_cache::IteratorList itList;
    auto cache = diff_cache::Create(data, itList);

    ASSERT_EQ(1, cache->size());
    auto node = cache->begin();

    ASSERT_EQ(data.at(0), node->first.Info.Byte);
    ASSERT_EQ(0, node->first.Info.Offset);

    ASSERT_EQ(0, node->second.Offset);
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
    diff_cache::IteratorList itList;
    auto cache = diff_cache::Create(data, itList);

    ASSERT_EQ(2, cache->size());

    auto aTree = cache->find(diff_cache::Key(0, 'a'));
    ASSERT_NE(cache->end(), aTree);

    auto BTree = cache->find(diff_cache::Key(0, 'B'));
    ASSERT_NE(cache->end(), BTree);

    // check B-tree
    ASSERT_EQ('B', BTree->first.Info.Byte);
    ASSERT_EQ(0, BTree->first.Info.Offset);

    ASSERT_EQ(data.find('B'), BTree->second.Offset);
    ASSERT_FALSE(BTree->second.DiffStrings);
    ASSERT_FALSE(BTree->second.SubStrings);

    // check a-tree
    ASSERT_EQ('a', aTree->first.Info.Byte);
    ASSERT_EQ(0, aTree->first.Info.Offset);

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
    diff_cache::IteratorList itList;
    auto cache = diff_cache::Create(data, itList);

    ASSERT_EQ(2, cache->size());

    auto aTree = cache->find(diff_cache::Key(0, 'a'));
    ASSERT_NE(cache->end(), aTree);

    auto BTree = cache->find(diff_cache::Key(0, 'B'));
    ASSERT_NE(cache->end(), BTree);

    // check a-tree diff str tree
    ASSERT_EQ(3, aTree->second.DiffStrings->size());

    uint32_t Offset = 3;
    size_t diffStrStartOffset = 1;
    while (Offset != 0)
    {
        auto diffStr = aTree->second.DiffStrings->find(diff_cache::Key(Offset, 'B'));
        ASSERT_NE(aTree->second.DiffStrings->end(), diffStr);

        ASSERT_EQ(diffStrStartOffset, diffStr->second.Offset);
        ASSERT_FALSE(diffStr->second.DiffStrings);
        ASSERT_FALSE(diffStr->second.SubStrings);

        size_t i = diffStr->second.Offset;
        for (; i < diffStr->second.Offset + diffStr->first.Info.Offset; ++i)
        {
            ASSERT_EQ('a', data.at(i));
        }

        ASSERT_EQ('B', data.at(i));

        --Offset;
        ++diffStrStartOffset;
    }
}