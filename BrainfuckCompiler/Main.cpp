#include <fstream>
#include <iostream>
#include "Assert.hpp"
#include "IO.hpp"
#include "InstructionDump.hpp"
#include "JIT/ProgramJIT.hpp"
#include "OptimizationPassManager.hpp"
#include "Passes/ArithmeticMergePass.hpp"
#include "Passes/OptimizeClearloopsPass.hpp"
#include "Passes/OptimizeCopyloopsPass.hpp"
#include "Passes/ReducePointerMovesPass.hpp"
#include "Program.hpp"
#include "ProgramInterpreter.hpp"

int main() {
  std::ifstream file("mandelbrot.bf");

  Assert(file && "Failed to open file.");

  auto program = bf::ParseProgram(file);

  bf::opt::OptimizationPassManager passManager;

  passManager.Add<bf::passes::ReducePointerMovesPass>();
  passManager.Add<bf::passes::OptimizeClearloopsPass>();
  passManager.Add<bf::passes::OptimizeCopyloopsPass>();
  passManager.Add<bf::passes::ArithmeticMergePass>();

  while (passManager.RunOnProgram(program, true)) {
    //
  }

  if (false) {
    bf::DumpProgram(std::cout, program);
  }

  constexpr auto bufferSize = 300'000;

  bf::jit::RunProgramJITed(program, bufferSize);
  // bf::interpreter::RunProgramInterpreted(program, bufferSize);

  bf::io::Flush();
}