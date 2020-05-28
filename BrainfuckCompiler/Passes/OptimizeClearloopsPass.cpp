#include "OptimizeClearloopsPass.hpp"

void bf::passes::OptimizeClearloopsPass::Initialize() {}

bool Match(const bf::Instruction* instruction, size_t size) {
  return true;
}

template <typename T, typename... Args>
bool Match(const bf::Instruction* instruction,
           size_t size,
           T* value,
           Args... args) {
  if (size == 0) {
    return false;
  }

  const auto inside = std::get_if<T>(instruction);
  if (!inside) {
    return false;
  }

  *value = *inside;

  return Match(instruction + 1, size - 1, args...);
}

bf::Program bf::passes::OptimizeClearloopsPass::Optimize(bf::Program program) {
  bf::Program optimizedProgram;
  optimizedProgram.reserve(program.size());

  for (size_t i = 0; i < program.size(); ++i) {
    const auto sizeLeft = program.size() - i;
    const auto instruction = &program[i];

    LoopStart ls;
    ModifyValue mv;
    LoopEnd le;

    if (Match(instruction, sizeLeft, &ls, &mv, &le)) {
      if (std::abs(mv.difference) == 1) {
        optimizedProgram.emplace_back(SetValue{mv.offset, 0});
        i += 2;

        continue;
      }
    }

    optimizedProgram.emplace_back(*instruction);
  }

  return optimizedProgram;
}

const std::string& bf::passes::OptimizeClearloopsPass::GetPassName() const {
  static std::string passName = "Optimize clearloop pass";

  return passName;
}
