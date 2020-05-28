#include "OptimizationPassManager.hpp"
#include <iostream>

void bf::opt::OptimizationPassManager::RunOnProgram(bf::Program& program,
                                                    bool debugLog) {
  auto runPass = [&](OptimizationPass* pass) {
    pass->Initialize();

    const auto instructionsBefore = program.size();

    program = pass->Optimize(std::move(program));

    const auto instructionsAfter = program.size();

    if (debugLog) {
      std::cout << "Ran " << pass->GetPassName() << " optimization pass.\n";
      std::cout << "  " << instructionsBefore << " -> " << instructionsAfter
                << "\n";
    }
  };

  for (const auto& pass : passes) {
    runPass(pass.get());
  }
}
