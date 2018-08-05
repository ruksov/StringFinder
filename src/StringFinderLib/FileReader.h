#pragma once
#include "IReader.h"

namespace sf::lib
{
    class FileReader : IReader
    {
    public:
        FileReader(std::wstring filePath, size_t dataSize = 100'000'000);

        void ResetRead() override;
        bool HasNext() const noexcept override;
        bool ReadNext(Data& data) override;

    private:
        void ResetImpl();

    private:
        std::ifstream m_file;
        size_t m_dataCount = 0;
        size_t m_dataSize = 0;
        size_t m_lastDataSize = 0;
    };
}


