// TestStringFinderLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <src/gtest-all.cc>
#include <src/gmock-all.cc>

int main(int argc, wchar_t **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}