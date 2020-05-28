#include "ProgramJIT.hpp"
#include "Compiler.hpp"

void bf::jit::RunProgramJITed(const Program& program, size_t bufferSize) {
  jit::Compiler compiler(program);

  compiler.Compile();
  compiler.Run(bufferSize);
}
