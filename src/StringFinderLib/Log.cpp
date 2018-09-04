#include "stdafx.h"
#include "Log.h"

namespace sf::lib
{
    void LogThreadSafe(std::wstringstream ss)
    {
        static std::mutex m;
        std::lock_guard lock(m);
        std::wcout <<__TIME__ << " " << ss.str() << '\n';
    }
}