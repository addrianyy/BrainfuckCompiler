#pragma once
#include "../LinkedLoops.hpp"
#include "../Program.hpp"
#include "ByteBuffer.hpp"

namespace bf::jit {

struct LoopInstance {
  uint64_t head;
  uint64_t jz;
};

class JITCompiler {
  Program program;
  LinkedLoops loops;

  ByteBuffer compiledCode;

  std::vector<LoopInstance> loopStack;

  void CompileInstruction(const Instruction& instruction);

 public:
  JITCompiler(Program program)
      : program(std::move(program)), loops(*LinkedLoops::Link(program)) {}

  void Compile();
  void Run();
};

}  // namespace bf::jit