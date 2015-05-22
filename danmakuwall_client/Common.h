#pragma once

// by µÛÇò from YSLib
#if (__cplusplus >= 201103L || (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + \
	__GNUC_PATCHLEVEL__) >= 40600)
	#define ynothrow noexcept
#elif (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >= 30300
	#define ynothrow __attribute__((nothrow))
#else
	#define ynothrow throw()
#endif

// STL
#include <array>
#include <memory>
#include <string>
#include <deque>
#include <list>
#include <queue>
#include <unordered_map>
#include <regex>
#include <atomic>

// fancy2d
#include <fcyIO/fcyStream.h>
#include <fcyParser/fcyJson.h>
#include <fcyMisc/fcyStringHelper.h>
#include <fcyOS/fcyDebug.h>
#include <fcyOS/fcyMultiThread.h>
#include <fcyRefObj.h>
#include <f2d.h>
