#pragma once
#include <sstream>
#include <filesystem>

namespace fs = std::experimental::filesystem;

#ifdef _DEBUG
#define LOG_DEBUG(__msg__)                          \
{                                                   \
    std::wstringstream ss;                          \
    ss << __TIME__ << " [DEBUG] "                   \
        << fs::path(__FILE__).filename()            \
        << "(" << __LINE__ << "), "                 \
        << __FUNCTION__ << ": " << __msg__;         \
    sf::lib::LogThreadSafe(std::move(ss));          \
}
#else
#define LOG_DEBUG(...)
#endif  //_DEBUG

#define LOG_INFO(__msg__)                           \
{                                                   \
    std::wstringstream ss;                          \
    ss << __TIME__ << " [INFO]: " << __msg__;       \
    sf::lib::LogThreadSafe(std::move(ss));          \
}

#define LOG_ERROR_EX(__msg__)                       \
{                                                   \
    std::wstringstream ss;                          \
    ss << __TIME__ << " [ERROR] "                   \
        << fs::path(__FILE__).filename()            \
        << "(" << __LINE__ << "), "                 \
        << __FUNCTION__ << ": " << __msg__;         \
    sf::lib::LogThreadSafe(std::move(ss));          \
}

#define LOG_ERROR(__msg__)                          \
{                                                   \
    std::wstringstream ss;                          \
    ss << __TIME__ << " [ERROR]: " << __msg__;      \
    sf::lib::LogThreadSafe(std::move(ss));          \
}

namespace sf::lib
{
    void LogThreadSafe(std::wstringstream ss);
}