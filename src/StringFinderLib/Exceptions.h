#pragma once
#include <sstream>
#include <filesystem>

namespace fs = std::experimental::filesystem;

#define THROW(__msg__)                         \
{                                               \
    std::stringstream st;                       \
    st << fs::path(__FILE__).filename()         \
        << "(" << __LINE__ << "), "             \
        << __FUNCTION__ << ": " << __msg__;     \
    throw std::runtime_error(st.str()); \
}

#define THROW_IF(__cond__, __msg__) if (__cond__)  \
{                                                   \
    std::stringstream st;                           \
        st << fs::path(__FILE__).filename()         \
        << "(" << __LINE__ << "), "                 \
        << __FUNCTION__ << ": " << __msg__;         \
    throw std::runtime_error(st.str());             \
}