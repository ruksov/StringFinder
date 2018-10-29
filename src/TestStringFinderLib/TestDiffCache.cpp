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

        std::string GetDataSubStr(const CacheMatchResult& res)
        {
            return m_data.substr(res.CacheOffset, res.MatchLen);
        }

        std::string GetCmpDataSubStr(const CacheMatchResult& res)
        {
            return m_cmpData.substr(res.CmpDataOffset, res.MatchLen);
        }

        bool CompareData(const CacheMatchResult& res)
        {
            return GetDataSubStr(res) == GetCmpDataSubStr(res);
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

    m_cmpOffset = 1;
    ASSERT_GT(m_cmpOffset, m_cmpData.size());
    ASSERT_FALSE(m_cache->GetFirstResult(m_cmpOffset, m_cmpData));
}

TEST_F(TestDiffCache, GetFirstResult_WithOneMatch)
{
    m_data = "some_data";
    ASSERT_NO_THROW(CreateCache());

    m_cmpOffset = 0;
    m_cmpData = "some";
    auto res = m_cache->GetFirstResult(m_cmpOffset, m_cmpData);
    
    ASSERT_TRUE(res);
    ASSERT_STREQ("some", GetDataSubStr(res.value()).data());
}

TEST_F(TestDiffCache, GetFirstResult_WithTwoMatches)
{
    m_data = "some_data_some";
    ASSERT_NO_THROW(CreateCache());

    m_cmpOffset = 0;
    m_cmpData = "some";
    auto res = m_cache->GetFirstResult(m_cmpOffset, m_cmpData);

    ASSERT_TRUE(res);
    ASSERT_EQ(0, res->CmpDataOffset);
    ASSERT_STREQ("some", GetDataSubStr(res.value()).data());

    // try repeat call
    res = m_cache->GetFirstResult(m_cmpOffset, m_cmpData);

    ASSERT_TRUE(res);
    ASSERT_EQ(0, res->CmpDataOffset);
    ASSERT_STREQ("some", GetDataSubStr(res.value()).data());
}

TEST_F(TestDiffCache, GetNextResult_OutOfRange)
{
    m_data = "some_data";
    m_cmpData = "data";
    ASSERT_NO_THROW(CreateCache());

    // wrong cache offset
    CacheMatchResult prevRes(42, 0, 0);
    ASSERT_GT(prevRes.CacheOffset, m_data.size());
    ASSERT_ANY_THROW(m_cache->GetNextResult(prevRes, m_cmpData));

    // wrong compare data offset
    prevRes = CacheMatchResult(0, 42, 0);
    ASSERT_GT(prevRes.CmpDataOffset, m_cmpData.size());
    auto res = m_cache->GetNextResult(prevRes, m_cmpData);
    ASSERT_FALSE(res);

    // wrong match length
    prevRes = CacheMatchResult(0, 0, 42);
    prevRes.MatchLen = 42;
    ASSERT_GT(prevRes.CmpDataOffset + prevRes.MatchLen, m_cmpData.size());
    res = m_cache->GetNextResult(prevRes, m_cmpData);
    ASSERT_FALSE(res);
}

TEST_F(TestDiffCache, GetNextResult_FalseNextMatch)
{
    m_data = "some_data_some";
    ASSERT_NO_THROW(CreateCache());

    // get first match result
    m_cmpData = "datadata";
    auto res = m_cache->GetFirstResult(m_cmpOffset, m_cmpData);
    ASSERT_TRUE(res);
    ASSERT_TRUE(CompareData(res.value()));

    // must return false result
    ASSERT_FALSE(m_cache->GetNextResult(res.value(), m_cmpData));
}

TEST_F(TestDiffCache, GetNextResult_HasNextMatch)
{
    m_data = "some_data_datadata";
    ASSERT_NO_THROW(CreateCache());

    // get first match result
    m_cmpData = "datadata";
    auto res = m_cache->GetFirstResult(m_cmpOffset, m_cmpData);
    ASSERT_TRUE(res);
    ASSERT_TRUE(CompareData(res.value()));

    // get next match result
    auto nextRes = m_cache->GetNextResult(res.value(), m_cmpData);
    ASSERT_TRUE(nextRes);
    ASSERT_GT(nextRes->MatchLen, res->MatchLen);
    ASSERT_EQ(m_cmpData.size(), nextRes->MatchLen);

    ASSERT_TRUE(CompareData(nextRes.value()));
    ASSERT_STREQ("datadata", GetDataSubStr(nextRes.value()).data());

    // must return false result
    ASSERT_FALSE(m_cache->GetNextResult(nextRes.value(), m_cmpData));
}

TEST_F(TestDiffCache, GetNextResult_UpdatePrevResult_False)
{
    m_data = "aaaaabbbbb";
    ASSERT_NO_THROW(CreateCache());

    m_cmpData = "bbaacccc";
    const CacheMatchResult finalExpectRes(0, 2, 2);

    // get first match result
    auto cmpDataPart = m_cmpData.substr(0, 4);
    auto testRes = m_cache->GetFirstResult(finalExpectRes.CmpDataOffset, cmpDataPart);
    ASSERT_TRUE(testRes);
    ASSERT_EQ(finalExpectRes, testRes.value());

    // virtualize cmp data offset
    testRes->CmpDataOffset = 0 - testRes->MatchLen;

    // update result
    cmpDataPart = m_cmpData.substr(4, 4);
    ASSERT_FALSE(m_cache->GetNextResult(testRes.value(), cmpDataPart));
}

TEST_F(TestDiffCache, GetNextResult_UpdatePrevResult_True_NullSubDiffTree)
{
    m_data = "aaaaabbbbb";
    ASSERT_NO_THROW(CreateCache());

    m_cmpData = "bbaaaaca";
    const CacheMatchResult finalExpectRes(0, 2, 4);

    // get first match result
    CacheMatchResult expectRes(0, 2, 2);
    auto cmpDataPart = m_cmpData.substr(0, 4);
    auto testRes = m_cache->GetFirstResult(expectRes.CmpDataOffset, cmpDataPart);
    ASSERT_TRUE(testRes);
    ASSERT_EQ(expectRes, testRes.value());

    // virtualize cmp data offset
    testRes->CmpDataOffset = 0 - testRes->MatchLen;

    // update result
    cmpDataPart = m_cmpData.substr(4, 4); 
    testRes = m_cache->GetNextResult(testRes.value(), cmpDataPart);
    ASSERT_TRUE(testRes);
    ASSERT_EQ('c', cmpDataPart.at(testRes->CmpDataOffset + testRes->MatchLen));

    testRes->CmpDataOffset = finalExpectRes.CmpDataOffset;
    ASSERT_EQ(finalExpectRes, testRes.value());
}

TEST_F(TestDiffCache, GetNextResult_UpdatePrevResult_True_CmpDataEnd)
{
    m_data = "aaaaabbbbb";
    ASSERT_NO_THROW(CreateCache());

    m_cmpData = "bbaaaa";
    const CacheMatchResult finalExpectRes(0, 2, 4);

    // get first match result
    CacheMatchResult expectRes(0, 2, 2);
    auto cmpDataPart = m_cmpData.substr(0, 4);
    auto testRes = m_cache->GetFirstResult(expectRes.CmpDataOffset, cmpDataPart);
    ASSERT_TRUE(testRes);
    ASSERT_EQ(expectRes, testRes.value());

    // virtualize cmp data offset
    testRes->CmpDataOffset = 0 - testRes->MatchLen;

    // update result
    cmpDataPart = m_cmpData.substr(4, 2);
    testRes = m_cache->GetNextResult(testRes.value(), cmpDataPart);
    ASSERT_TRUE(testRes);
    ASSERT_EQ(cmpDataPart.size(), testRes->CmpDataOffset + testRes->MatchLen);

    testRes->CmpDataOffset = finalExpectRes.CmpDataOffset;
    ASSERT_EQ(finalExpectRes, testRes.value());
}

TEST_F(TestDiffCache, GetNextResult_FindParentIterator)
{
    m_data = "aaaaabcaaad";
    ASSERT_NO_THROW(CreateCache());

    m_cmpData = "caaaaab";
    const CacheMatchResult finalExpectRes(0, 1, 6);

    // get first match result
    CacheMatchResult tmpExpectRes(m_data.find('c'), 0, 4);
    auto testRes = m_cache->GetFirstResult(0, m_cmpData);
    ASSERT_TRUE(testRes);
    ASSERT_EQ(tmpExpectRes, testRes.value());

    // create prev res to check possibility for optimization
    CacheMatchResult optimizeRes(testRes->CacheOffset + 1, testRes->CmpDataOffset + 1, testRes->MatchLen - 1);
    testRes = m_cache->GetNextResult(optimizeRes, m_cmpData);
    ASSERT_TRUE(testRes);
    ASSERT_EQ(finalExpectRes, testRes.value());
}