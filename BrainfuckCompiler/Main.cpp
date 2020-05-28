#include <fstream>
#include "JIT/ProgramJIT.hpp"
#include "OptimizationPass.hpp"
#include "OptimizationPassManager.hpp"
#include "Passes/ArithmeticMegePass.hpp"
#include "Passes/ReducePointerMovesPass.hpp"
#include "Program.hpp"
#include "ProgramInterpreter.hpp"

int main() {
  std::ifstream file("mandelbrot.bf");

  auto program = bf::ParseProgram(file);

  bf::opt::OptimizationPassManager passManager;

  passManager.Add<bf::passes::ArithmeticMergePass>();
  passManager.Add<bf::passes::ReducePointerMovesPass>();

  for (int i = 0; i < 2; ++i) {
    passManager.RunOnProgram(program);
  }

  const auto bufferSize = 300'000;

  bf::jit::RunProgramJITed(program, bufferSize);
  // bf::interpreter::RunProgramInterpreted(program, bufferSize);
}