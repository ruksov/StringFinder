#include "stdafx.h"
#include "FileReader.h"
#include "Exceptions.h"

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
        m_file.open(filePath, std::ios::in | std::ios::binary);

        ResetImpl();
    }

    void FileReader::Reset()
    {
        ResetImpl();
    }

    bool FileReader::HasNext() const noexcept
    {
        return m_dataCount != 0 || m_lastDataSize != 0;
    }

    bool FileReader::ReadNext(Data & data)
    {
        auto res = false;

        if (m_dataCount)
        {
            data.resize(m_dataSize);
            m_file.read(data.data(), m_dataSize);
            --m_dataCount;
            res = true;
        }
        else if (m_lastDataSize)
        {
            data.resize(m_lastDataSize);
            m_file.read(data.data(), m_lastDataSize);
            m_lastDataSize = 0;
            res = true;
        }
        else
        {
            data.resize(0);
        }

        return res;
    }

    void FileReader::ResetImpl()
    {
        m_file.seekg(0, m_file.end);
        const auto fileSize = static_cast<size_t>(m_file.tellg());

        //
        // Reset variables, whcih keeps information about dividing file data
        //
        m_dataCount = fileSize / m_dataSize;
        m_dataSize = m_dataSize;
        m_lastDataSize = fileSize % m_dataSize;

        m_file.seekg(0);
    }
}