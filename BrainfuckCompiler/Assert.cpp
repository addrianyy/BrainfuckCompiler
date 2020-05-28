#include "Assert.hpp"
#include <Windows.h>
#include <iostream>

void AssertInternal(bool condition,
                    const char* conditionText,
                    const char* filename,
                    uint32_t lineNumer) {
  if (!condition) {
    std::cerr << "Assertion '" << conditionText << "' failed.\n";
    std::cerr << "File: " << filename << ". Line: " << lineNumer << ".\n";

    if (IsDebuggerPresent()) {
      DebugBreak();
    }

    exit(-1);
  }
}
