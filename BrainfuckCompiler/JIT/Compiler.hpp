#pragma once
#include "../Program.hpp"
#include "ByteBuffer.hpp"

namespace bf::jit {

struct LoopInstance {
  uint64_t condition;
  uint64_t jumpToTheEnd;
};

class Compiler {
  Program program;
  ByteBuffer compiledCode;

  std::vector<LoopInstance> loopStack;

  void CompileInstruction(const Instruction& instruction);

 public:
  Compiler(Program program) : program(std::move(program)) {}

  void Compile();
  void Run(size_t bufferSize);
};

}  // namespace bf::jit