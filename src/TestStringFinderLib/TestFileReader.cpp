#include "stdafx.h"
#include "FileReader.h"

using namespace testing;

namespace 
{
    struct TestFileReader : Test
    {
    public:
        size_t GetFileSize()
        {
            m_file.seekp(0, m_file.end);
            return m_file.tellp();
        }

        void FillFile(const std::string& data)
        {
            m_file.seekp(0);
            m_file << data;
            m_file.close();

            m_file.open(m_path, std::ios::app | std::ios::binary | std::ios::out);
            ASSERT_TRUE(m_file.is_open());
        }

    protected:
        void SetUp() override
        {
            m_file.open(m_path, std::ios::binary | std::ios::out);
            ASSERT_TRUE(m_file.is_open());
        }

        void TearDown() override
        {
            m_file.close();
            m_reader.reset();
            ASSERT_EQ(0, remove(m_path.c_str()));
        }

    protected:
        std::ofstream m_file;
        std::string m_path = "test.txt";
        size_t m_fileSize = 0;
        size_t m_dataSize = 0;
        std::unique_ptr<sf::lib::IReader> m_reader;
    };
}

TEST_F(TestFileReader, FileOpenError)
{
    ASSERT_THROW(m_reader.reset(new sf::lib::FileReader("errorPath.txt", 10)), std::runtime_error);
}

TEST_F(TestFileReader, DataSizeEqualZero)
{
    FillFile("test");
    ASSERT_THROW(m_reader.reset(new sf::lib::FileReader(m_path.c_str(), 0)), std::runtime_error);
}

TEST_F(TestFileReader, ReadEmptyFile)
{
    ASSERT_NO_THROW(m_reader.reset(new sf::lib::FileReader(m_path.c_str(), m_dataSize)));

    std::string data;
    auto prevSize = data.size();

    ASSERT_TRUE(m_reader->IsEnd());
    ASSERT_THROW(m_reader->ReadNext(data), std::runtime_error);
    ASSERT_EQ(0, m_reader->GetIndex());
    ASSERT_EQ(prevSize, data.size());
}

TEST_F(TestFileReader, AlignedFileSize)
{
    m_dataSize = 2;

    std::string fileData = "1234";
    FillFile(fileData);
    ASSERT_EQ(0, GetFileSize() % m_dataSize);

    ASSERT_NO_THROW(m_reader.reset(new sf::lib::FileReader(m_path.c_str(), m_dataSize)));

    std::string resData;
    std::string data;

    size_t index = 0;
    while (!m_reader->IsEnd())
    {
        ASSERT_EQ(index, m_reader->GetIndex());
        ASSERT_NO_THROW(m_reader->ReadNext(data));
        ASSERT_EQ(m_dataSize, data.size());
        resData += data;
        ++index;
    }

    ASSERT_STREQ(fileData.c_str(), resData.c_str());

    ASSERT_TRUE(m_reader->IsEnd());
    ASSERT_EQ(index, m_reader->GetIndex());
    
    auto prevData = data;
    ASSERT_THROW(m_reader->ReadNext(data), std::runtime_error);
    ASSERT_STREQ(prevData.c_str(), data.c_str());
}

TEST_F(TestFileReader, NotAlignedFileSize)
{
    m_dataSize = 2;

    std::string fileData = "12345";
    FillFile(fileData);
    auto lastDataSize = GetFileSize() % m_dataSize;
    ASSERT_NE(0, lastDataSize);

    ASSERT_NO_THROW(m_reader.reset(new sf::lib::FileReader(m_path.c_str(), m_dataSize)));

    std::string resData;
    std::string data;

    size_t index = 0;
    while (!m_reader->IsEnd())
    {
        ASSERT_EQ(index, m_reader->GetIndex());
        ASSERT_NO_THROW(m_reader->ReadNext(data));

        if (resData.size() == GetFileSize() - lastDataSize)
        {
            //
            // Read last not aligned data chunk
            //
            ASSERT_EQ(lastDataSize, data.size());
        }
        else
        {
            ASSERT_EQ(m_dataSize, data.size());
        }

        resData += data;
        ++index;
    }

    ASSERT_STREQ(fileData.c_str(), resData.c_str());

    ASSERT_TRUE(m_reader->IsEnd());
    ASSERT_EQ(index, m_reader->GetIndex());

    auto prevData = data;
    ASSERT_THROW(m_reader->ReadNext(data), std::runtime_error);
    ASSERT_STREQ(prevData.c_str(), data.c_str());
}

TEST_F(TestFileReader, TestReset)
{
    m_dataSize = 2;

    std::string fileData = "12";
    FillFile(fileData);
    ASSERT_EQ(0, GetFileSize() % m_dataSize);

    ASSERT_NO_THROW(m_reader.reset(new sf::lib::FileReader(m_path.c_str(), m_dataSize)));

    std::string data1;
    ASSERT_FALSE(m_reader->IsEnd());
    ASSERT_EQ(0, m_reader->GetIndex());
    ASSERT_NO_THROW(m_reader->ReadNext(data1));
    
    ASSERT_EQ(1, m_reader->GetIndex());
    ASSERT_EQ(m_dataSize, data1.size());
    ASSERT_STREQ(fileData.c_str(), data1.c_str());

    ASSERT_TRUE(m_reader->IsEnd());

    //
    // Reset file reader to read from start file
    //
    ASSERT_NO_THROW(m_reader->Reset());
    
    std::string data2;
    ASSERT_FALSE(m_reader->IsEnd());
    ASSERT_EQ(0, m_reader->GetIndex());
    ASSERT_NO_THROW(m_reader->ReadNext(data2));

    ASSERT_EQ(1, m_reader->GetIndex());
    ASSERT_EQ(m_dataSize, data2.size());
    ASSERT_STREQ(fileData.c_str(), data2.c_str());

    ASSERT_TRUE(m_reader->IsEnd());

    ASSERT_EQ(data1.size(), data2.size());
    ASSERT_STREQ(data1.c_str(), data2.c_str());
}