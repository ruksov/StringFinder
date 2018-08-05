#pragma once
#include <sstream>

#define THROW(__mess__)                 \
{                                       \
    std::stringstream st;               \
    st << __mess__;                     \
    throw std::runtime_error(st.str()); \
}

#define THROW_IF(__cond__, __mess__) if (__cond__)  \
{                                       \
    std::stringstream st;               \
    st << __mess__;                     \
    throw std::runtime_error(st.str()); \
}