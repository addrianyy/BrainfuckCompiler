#include "ArithmeticMergePass.hpp"
#include <optional>

void bf::passes::ArithmeticMergePass::Initialize() {}

bf::Program bf::passes::ArithmeticMergePass::Optimize(bf::Program program) {
  using namespace instrs;

  bf::Program optimizedProgram;
  optimizedProgram.reserve(program.size());

  std::optional<bf::Instruction> streakInstruction;

  const auto writeStreakInstruction = [&]() {
    if (streakInstruction.has_value()) {
      optimizedProgram.emplace_back(*streakInstruction);
      streakInstruction = std::nullopt;
    }
  };

  for (const auto& instruction : program) {
    if (streakInstruction.has_value() &&
        streakInstruction->index() != instruction.index()) {
      writeStreakInstruction();
    }

    if (const auto mp = std::get_if<ModifyPointer>(&instruction)) {
      if (!streakInstruction.has_value()) {
        streakInstruction = *mp;
        continue;
      }

      auto& streak = std::get<ModifyPointer>(*streakInstruction);

      streak.offset += mp->offset;

      continue;
    }

    if (const auto mv = std::get_if<ModifyValue>(&instruction)) {
      if (!streakInstruction.has_value()) {
        streakInstruction = *mv;
        continue;
      }

      auto& streak = std::get<ModifyValue>(*streakInstruction);

      if (streak.offset == mv->offset) {
        streak.difference += mv->difference;
        continue;
      }

      writeStreakInstruction();

      streakInstruction = *mv;

      continue;
    }

    writeStreakInstruction();

    optimizedProgram.emplace_back(instruction);
  }

  writeStreakInstruction();

  return optimizedProgram;
}

const std::string& bf::passes::ArithmeticMergePass::GetPassName() const {
  static std::string passName = "Arithmetic merge pass";

  return passName;
}
