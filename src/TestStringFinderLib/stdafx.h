// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <fstream>
#include <future>
#include <array>
#include <vector>
#include <map>

#include <memory>
#include <optional>

#define GTEST_LANG_CXX11 1
#define GTEST_HAS_TR1_TUPLE 0 
#include <gtest/gtest.h>
#include <gmock/gmock.h>