#pragma once
#include <cstdint>

// Assert working on release builds.

void AssertInternal(bool condition,
                    const char* conditionText,
                    const char* filename,
                    uint32_t lineNumer);

#define MAKE_STRING(s) MAKE_STRING_1(s)
#define MAKE_STRING_1(s) #s

#define Assert(x) AssertInternal((x), MAKE_STRING(x), __FILE__, __LINE__)