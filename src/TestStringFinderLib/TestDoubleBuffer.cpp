#include "stdafx.h"
#include "DoubleBuffer.h"

using namespace testing;
using Data = sf::lib::Data;
namespace
{
    struct MockReader : sf::lib::IReader
    {
        MOCK_METHOD0(Reset, void());
        MOCK_METHOD1(ReadNext, void(Data&));
        MOCK_CONST_METHOD0(IsEnd, bool());
        MOCK_CONST_METHOD0(GetIndex, size_t());
        MOCK_CONST_METHOD0(GetDataCount, size_t());
        MOCK_CONST_METHOD0(GetDataSize, size_t());
    };
}

TEST(TestDoubleBuffer, EmptyReader)
{
    auto mock = std::make_unique<MockReader>();
    EXPECT_CALL(*mock, IsEnd())
        .WillRepeatedly(Return(true));

    sf::lib::DoubleBuffer buffer(std::move(mock));

    auto data = buffer.GetNext();
    ASSERT_EQ(0, data.get().size());
    ASSERT_EQ(0, buffer.GetIndex());

    auto data2 = buffer.GetNext();
    ASSERT_EQ(0, data2.get().size());
    ASSERT_EQ(0, buffer.GetIndex());
}

TEST(TestDoubleBuffer, OneRead)
{
    auto mock = std::make_unique<MockReader>();
    EXPECT_CALL(*mock, IsEnd())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    Data readData = "1234";
    EXPECT_CALL(*mock, ReadNext(_))
        .WillOnce(SetArgReferee<0>(readData));

    sf::lib::DoubleBuffer buffer(std::move(mock));

    auto data = buffer.GetNext();
    ASSERT_EQ(0, buffer.GetIndex());
    ASSERT_STREQ(readData.c_str(), data.get().c_str());

    auto data2 = buffer.GetNext();
    ASSERT_EQ(1, buffer.GetIndex());
    ASSERT_EQ(0, data2.get().size());
}

TEST(TestDoubleBuffer, SeveralReads)
{
    auto mock = std::make_unique<MockReader>();
    EXPECT_CALL(*mock, IsEnd())
        .Times(AtLeast(3))
        .WillOnce(Return(false))
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    Data readData = "12345678";
    EXPECT_CALL(*mock, ReadNext(_))
        .WillOnce(SetArgReferee<0>(readData.substr(0, 4)))
        .WillOnce(SetArgReferee<0>(readData.substr(4, 4)));

    sf::lib::DoubleBuffer buffer(std::move(mock));

    auto data = buffer.GetNext();
    uint8_t index = 0;
    while (data.get().size() != 0)
    {
        ASSERT_EQ(index, buffer.GetIndex());
        ASSERT_STREQ(readData.substr(index * 4, 4).c_str(), data.get().c_str());

        data = buffer.GetNext();
        ++index;
    }

    data = buffer.GetNext();
    ASSERT_EQ(0, data.get().size());
    ASSERT_EQ(index, buffer.GetIndex());
}

TEST(TestDoubleBuffer, TestReset)
{
    auto mock = std::make_unique<MockReader>();
    EXPECT_CALL(*mock, IsEnd())
        .WillOnce(Return(false))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*mock, Reset())
        .Times(1);

    Data readData = "1234";
    EXPECT_CALL(*mock, ReadNext(_))
        .WillOnce(SetArgReferee<0>(readData))
        .WillOnce(SetArgReferee<0>(readData));

    sf::lib::DoubleBuffer buffer(std::move(mock));

    auto data = buffer.GetNext();
    ASSERT_EQ(0, buffer.GetIndex());
    ASSERT_STREQ(readData.c_str(), data.get().c_str());

    auto data2 = buffer.GetNext();
    ASSERT_EQ(1, buffer.GetIndex());
    ASSERT_EQ(0, data2.get().size());

    // Reset buffer
    ASSERT_NO_THROW(buffer.Reset());

    ASSERT_EQ(0, buffer.GetIndex());

    data = buffer.GetNext();
    ASSERT_EQ(0, buffer.GetIndex());
    ASSERT_STREQ(readData.c_str(), data.get().c_str());

    data2 = buffer.GetNext();
    ASSERT_EQ(1, buffer.GetIndex());
    ASSERT_EQ(0, data2.get().size());
}