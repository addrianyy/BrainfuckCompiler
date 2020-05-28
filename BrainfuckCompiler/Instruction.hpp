#pragma once
#include <cstdint>
#include <variant>
#include <vector>

namespace bf {

struct ModifyPointer {
  int64_t offset;

  static ModifyPointer SingleAdd() { return ModifyPointer{1}; }
  static ModifyPointer SingleSubtract() { return ModifyPointer{-1}; }
};

struct ModifyValue {
  int64_t offset;
  int64_t difference;

  static ModifyValue SingleAdd() { return ModifyValue{0, 1}; }
  static ModifyValue SingleSubtract() { return ModifyValue{0, -1}; }
};

struct LoopStart {};

struct LoopEnd {};

struct ReadChar {
  int64_t offset;

  static ReadChar ReadCurrent() { return ReadChar{0}; }
};

struct WriteChar {
  int64_t offset;

  static WriteChar WriteCurrent() { return WriteChar{0}; }
};

using Instruction = std::variant<ModifyPointer,
                                 ModifyValue,
                                 LoopStart,
                                 LoopEnd,
                                 ReadChar,
                                 WriteChar>;
}  // namespace bf