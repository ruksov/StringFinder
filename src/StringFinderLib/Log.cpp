#include "stdafx.h"
#include "Log.h"

namespace sf::lib
{
    void LogThreadSafe(std::stringstream ss)
    {
        static std::mutex m;
        const auto t = std::time(nullptr);
        std::tm lt;
        localtime_s(&lt, &t);

        std::lock_guard lock(m);
        std::cout << std::put_time(&lt, "%T ") << ss.str() << '\n';
    }
}