#include "stdafx.h"
#include "FileReader.h"
#include "Exceptions.h"
#include "Log.h"

namespace sf::lib
{
    FileReader::FileReader(std::wstring filePath, size_t dataSize)
        : m_dataSize(dataSize)
    {
        //
        // Enable exception in logical and i/o errors
        //
        m_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        //
        // Open file in the end to compute file size
        //
        m_file.open(filePath, std::ios::in | std::ios::binary | std::ios::ate);

        const size_t fileSize = m_file.tellg();
        THROW_IF(dataSize == 0 && fileSize != 0,
            "Data size in file reader can't be equal 0, if file did not empty");

        if (fileSize != 0)
        {
            m_lastDataSize = fileSize % m_dataSize;
            m_dataCount = fileSize / m_dataSize + (m_lastDataSize != 0 ? 1 : 0);
        }

        LOG_DEBUG("Configure file reader:\n"
            << "\tData size: " << m_dataSize << "\n"
            << "\tData count: " << m_dataCount << '\n'
            << "\tLast data size: " << m_lastDataSize);

        m_file.seekg(0);
    }

    void FileReader::Reset()
    {
        m_index = 0;
        m_file.seekg(0);
    }

    void FileReader::ReadNext(Data & data)
    {
        THROW_IF(m_index >= m_dataCount, 
            "Failed to read next data. File reader riches to the end of file.");

        const auto dataSize = (m_lastDataSize != 0 && m_index == m_dataCount - 1) ? 
            m_lastDataSize : m_dataSize;

        data.resize(dataSize);
        m_file.read(data.data(), dataSize);
        ++m_index;
    }

    bool FileReader::IsEnd() const noexcept
    {
        return m_index >= m_dataCount;
    }

    size_t FileReader::GetIndex() const noexcept
    {
        return m_index;
    }

    size_t FileReader::GetDataCount() const noexcept
    {
        return m_dataCount;
    }
}