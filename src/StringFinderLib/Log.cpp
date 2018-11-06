#include "stdafx.h"
#include "Log.h"

namespace sf::log
{
    namespace config
    {
        static bool g_initLog = false;
        static std::mutex g_lock;
    }

    void InitLog()
    {
        config::g_initLog = true;
    }

    void LogThreadSafe(std::stringstream ss)
    {
        if (!config::g_initLog)
        {
            return;
        }

        const auto t = std::time(nullptr);
        std::tm lt;
        localtime_s(&lt, &t);

        std::lock_guard lock(config::g_lock);
        std::cout << std::put_time(&lt, "%T ") << ss.str() << '\n';
    }
}