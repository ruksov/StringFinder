#pragma once
#include "IReader.h"

namespace sf::lib
{
    class FileReader : public IReader
    {
    public:
        FileReader(std::string filePath, size_t dataSize);

        void Reset() override;
        void ReadNext(Data& data) override;
        bool IsEnd() const noexcept override;
        size_t GetIndex() const noexcept override;
        size_t GetDataCount() const noexcept override;

    private:
        std::ifstream m_file;
        size_t m_dataCount = 0;
        size_t m_dataSize = 0;
        size_t m_lastDataSize = 0;
        size_t m_index = 0;
    };
}


