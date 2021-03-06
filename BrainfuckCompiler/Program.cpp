#include "Program.hpp"
#include <unordered_map>
#include "LinkedLoops.hpp"

bf::Program bf::ParseProgram(std::istream& input) {
  using namespace instrs;

  const std::unordered_map<char, Instruction> mapping{
      {'>', ModifyPointer::SingleAdd()},
      {'<', ModifyPointer::SingleSubtract()},
      {'+', ModifyValue::SingleAdd()},
      {'-', ModifyValue::SingleSubtract()},
      {'[', LoopStart{}},
      {']', LoopEnd{}},
      {',', ReadChar::ReadCurrent()},
      {'.', WriteChar::WriteCurrent()},
  };

  Program program;

  char c = 0;
  while (input.get(c)) {
    const auto m = mapping.find(c);
    if (m != mapping.end()) {
      program.push_back(m->second);
    }
  }

  return program;
}