#pragma once
#include <cstdint>
#include <variant>
#include <vector>

namespace bf {

namespace instrs {

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

struct SetValue {
  int64_t offset;
  uint8_t value;
};

struct CopyAddValue {
  int64_t from;
  int64_t to;
  int64_t mult;
};

struct CopyValue {
  int64_t from;
  int64_t to;
};

}  // namespace instrs

using Instruction = std::variant<instrs::ModifyPointer,
                                 instrs::ModifyValue,
                                 instrs::LoopStart,
                                 instrs::LoopEnd,
                                 instrs::ReadChar,
                                 instrs::WriteChar,
                                 instrs::SetValue,
                                 instrs::CopyAddValue,
                                 instrs::CopyValue>;
}  // namespace bf