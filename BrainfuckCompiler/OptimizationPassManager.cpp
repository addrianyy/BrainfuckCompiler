#include "OptimizationPassManager.hpp"
#include <iostream>

bool bf::opt::OptimizationPassManager::RunOnProgram(bf::Program& program,
                                                    bool debugLog) {
  auto runPass = [&](OptimizationPass* pass) {
    pass->Initialize();

    const auto instructionsBefore = program.size();

    program = pass->Optimize(std::move(program));

    const auto instructionsAfter = program.size();

    if (debugLog && instructionsBefore != instructionsAfter) {
      if (instructionsBefore < instructionsAfter) {
        std::cout << pass->GetPassName() << " increased instruction count.\n";
        return;
      }

      const auto percent =
          float(instructionsBefore - instructionsAfter) / instructionsBefore;
      std::cout << pass->GetPassName() << ": reduced " << instructionsBefore
                << " to " << instructionsAfter << " (" << int(percent * 100)
                << "%)\n";
    }
  };

  const auto instructionsBefore = program.size();

  for (const auto& pass : passes) {
    runPass(pass.get());
  }

  const auto instructionsAfter = program.size();

  return instructionsBefore > instructionsAfter;
}
