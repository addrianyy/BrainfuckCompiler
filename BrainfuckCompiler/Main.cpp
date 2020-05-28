#include <chrono>
#include <fstream>
#include <iostream>
#include "IO.hpp"
#include "InstructionDump.hpp"
#include "JIT/ProgramJIT.hpp"
#include "OptimizationPass.hpp"
#include "OptimizationPassManager.hpp"
#include "Passes/ArithmeticMergePass.hpp"
#include "Passes/ComplexArithmeticMergePass.hpp"
#include "Passes/OptimizeClearloopsPass.hpp"
#include "Passes/OptimizeCopyloopsPass.hpp"
#include "Passes/ReducePointerMovesPass.hpp"
#include "Program.hpp"
#include "ProgramInterpreter.hpp"

int main() {
  std::ifstream file("mandelbrot.bf");

  auto program = bf::ParseProgram(file);

  bf::opt::OptimizationPassManager passManager;

  passManager.Add<bf::passes::ArithmeticMergePass>();
  passManager.Add<bf::passes::ReducePointerMovesPass>();
  passManager.Add<bf::passes::OptimizeClearloopsPass>();
  passManager.Add<bf::passes::ComplexArithmeticMergePass>();
  passManager.Add<bf::passes::OptimizeCopyloopsPass>();


  while (passManager.RunOnProgram(program, true)) {
  }

  for (int i = 0; i < 3; ++i) {
    passManager.RunOnProgram(program, true);
  }

  std::cin.get();


  const auto bufferSize = 300'000;

  //bf::DumpProgram(std::cout, program);

  bf::jit::RunProgramJITed(program, bufferSize);
  //bf::interpreter::RunProgramInterpreted(program, bufferSize);

  bf::io::Flush();
}