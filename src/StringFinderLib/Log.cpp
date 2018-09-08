#include "stdafx.h"
#include "Log.h"

namespace sf::lib
{
    void LogThreadSafe(std::wstringstream ss)
    {
        static std::mutex m;
        auto t = std::time(nullptr);
        std::tm lt;
        localtime_s(&lt, &t);

        std::lock_guard lock(m);
        std::wcout << std::put_time(&lt, L"%T ") << ss.str() << '\n';
    }
}