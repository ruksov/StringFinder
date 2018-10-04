#include "stdafx.h"
#include "ConsoleProgressBar.h"

namespace sf::lib
{
    constexpr size_t kMaxSharpCount = 30;
    constexpr size_t kPercentPerSharp = 100 / kMaxSharpCount;

    void ConsoleProgressBar(size_t current, size_t end)
    {
        const size_t percentCount = (current * 100) / end;
        const size_t sharpCount = percentCount / kPercentPerSharp;

        std::cout << "Progress: [";
        for (size_t i = 0; i < kMaxSharpCount; ++i)
        {
            std::cout << (i < sharpCount ? '#' : '.');
        }

        std::cout << "] " << percentCount << "%\r";
    }
}