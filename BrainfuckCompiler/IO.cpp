#include "IO.hpp"
#include <cstdio>

constexpr size_t BufferMaxSize = 10;

static size_t CurrentSize = 0;
static char Buffer[BufferMaxSize + 1];

void bf::io::Flush() {
  Buffer[CurrentSize] = 0;

  fputs(Buffer, stdout);

  CurrentSize = 0;
}

void bf::io::WriteChar(char c) {
  if (CurrentSize + 1 > BufferMaxSize) {
    Flush();
  }

  Buffer[CurrentSize++] = c;
}

char bf::io::ReadChar() {
  io::Flush();

  return char(std::getchar());
}
