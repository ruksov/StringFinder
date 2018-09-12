#include "stdafx.h"
#include "CacheFactory.h"

using namespace testing;
using namespace sf::lib;

namespace
{
    struct TestDiffCache : public Test
    {
    public:
        void CreateCache()
        {
            m_cache = CacheFactory(CacheType::DiffCache, m_data);
        }

    protected:
        Data m_data;
        Data m_cmpData;
        size_t m_cmpOffset = 0;
        CachePtr m_cache;
    };
}

TEST_F(TestDiffCache, EmptyCache)
{
    ASSERT_NO_THROW(CreateCache());
    ASSERT_TRUE(m_cache->GetCacheData().empty());
}

TEST_F(TestDiffCache, GetCacheData)
{
    m_data = "some_data";
    ASSERT_NO_THROW(CreateCache());
    ASSERT_STREQ(m_data.data(), m_cache->GetCacheData().data());
}

TEST_F(TestDiffCache, Reset)
{
    m_data = "some_data";
    ASSERT_NO_THROW(CreateCache());
    ASSERT_STREQ(m_data.data(), m_cache->GetCacheData().data());

    m_data = "another_data";
    ASSERT_NO_THROW(m_cache->Reset(m_data));
    ASSERT_STREQ(m_data.data(), m_cache->GetCacheData().data());
}

TEST_F(TestDiffCache, GetFirstResult_OutOfRange)
{
    m_data = "some_data";
    ASSERT_NO_THROW(CreateCache());

    CacheMatchResult inOutRes(0, 1, 0);
    ASSERT_GT(inOutRes.CmpDataOffset, m_cmpData.size());
    ASSERT_FALSE(m_cache->GetFirstResult(inOutRes, m_cmpData));
}

TEST_F(TestDiffCache, GetFirstResult_UpdateResult_True)
{
    m_data = "aaaabaaac";
    ASSERT_NO_THROW(CreateCache());

    m_cmpData = "aaaa";
    CacheMatchResult inOutRes(m_data.find('b') + 1, 0, 3);

    // inOutRes must updates to match with 0 offset in m_data
    // and update match length
    CacheMatchResult expectInOutRes(0, 0, 4);
    ASSERT_TRUE(m_cache->GetFirstResult(inOutRes, m_cmpData));
    ASSERT_EQ(expectInOutRes, inOutRes);
}

TEST_F(TestDiffCache, GetFirstResult_UpdateResult_False)
{
    m_data = "aaaabaaac";
    ASSERT_NO_THROW(CreateCache());

    m_cmpData = "aaaca";
    CacheMatchResult inOutRes(m_data.find('b') + 1, 0, 4);

    // inOutRes must has same data after call cache function
    CacheMatchResult expectInOutRes(m_data.find('b') + 1, 0, 4);
    ASSERT_FALSE(m_cache->GetFirstResult(inOutRes, m_cmpData));
    ASSERT_EQ(expectInOutRes, inOutRes);
}

TEST_F(TestDiffCache, GetFirstResult_UnspecifiedInOutRes_True)
{
    m_data = "aaaabbba";
    ASSERT_NO_THROW(CreateCache());

    m_cmpData = "aabbbc";
    CacheMatchResult inOutRes(0, 2, 0);

    CacheMatchResult expectInOutRes(4, 2, 3);
    ASSERT_TRUE(m_cache->GetFirstResult(inOutRes, m_cmpData));
    ASSERT_EQ(expectInOutRes, inOutRes);
}

TEST_F(TestDiffCache, GetFirstResult__UnspecifiedInOutRes_True)
{
    m_data = "aaaa";
    ASSERT_NO_THROW(CreateCache());

    m_cmpData = "bbbb";
    CacheMatchResult inOutRes(0, 0, 0);

    // first result not found
    CacheMatchResult expectInOutRes(0, 0, 0);
    ASSERT_FALSE(m_cache->GetFirstResult(inOutRes, m_cmpData));
    ASSERT_EQ(expectInOutRes, inOutRes);
}

TEST_F(TestDiffCache, GetNextResult_NoSubDiffTree)
{
    m_data = "aaaabaaaccc";
    ASSERT_NO_THROW(CreateCache());

    // match range: "baaa"
    m_cmpData = "baaaddd";
    CacheMatchResult expectRes(m_data.find('b'), 0, 4);

    CacheMatchResult actualRes = expectRes;
    ASSERT_FALSE(m_cache->GetNextResult(actualRes, m_cmpData));
    ASSERT_EQ(expectRes, actualRes);
}

TEST_F(TestDiffCache, GetNextResult_EndOfCmpData)
{
    m_data = "aabaaaccc";
    ASSERT_NO_THROW(CreateCache());

    // match range: "baaa"
    m_cmpData = "ddbaaa";
    CacheMatchResult expectRes(m_data.find('b'), m_cmpData.find('b'), 4);

    CacheMatchResult actualRes = expectRes;
    ASSERT_FALSE(m_cache->GetNextResult(actualRes, m_cmpData));
    ASSERT_EQ(expectRes, actualRes);
}

TEST_F(TestDiffCache, GetNextResult_NoRangeInSubDiffTree)
{
    m_data = "aabaaaccbaaadd";
    ASSERT_NO_THROW(CreateCache());

    // match range: "baaa"
    m_cmpData = "baaaee";
    CacheMatchResult expectRes(m_data.find('b'), 0, 4);

    CacheMatchResult actualRes = expectRes;
    ASSERT_FALSE(m_cache->GetNextResult(actualRes, m_cmpData));
    ASSERT_EQ(expectRes, actualRes);
}

TEST_F(TestDiffCache, GetNextResult_Success)
{
    m_data = "aabaaaccbaaaeeff";
    ASSERT_NO_THROW(CreateCache());

    // final match range: "baaaee"
    // cache data: aabaaaccBAAAEEff
    m_cmpData = "baaaeegg";
    CacheMatchResult expectRes(8, 0, 6);

    // first match range: "baaa"
    // cache data: aaBAAAccbaaaeeff
    CacheMatchResult actualRes(2, 0, 4);
    ASSERT_TRUE(m_cache->GetNextResult(actualRes, m_cmpData));
    ASSERT_EQ(expectRes, actualRes);
}