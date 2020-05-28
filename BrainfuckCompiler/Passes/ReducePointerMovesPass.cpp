#include "ReducePointerMovesPass.hpp"
#include "../Assert.hpp"
#include "../VisitorHelper.hpp"

void bf::passes::ReducePointerMovesPass::Initialize() {}

bf::Program bf::passes::ReducePointerMovesPass::Optimize(bf::Program program) {
  bf::Program optimizedProgram;
  optimizedProgram.reserve(program.size());

  int64_t currentOffset = 0;

  for (auto& instruction : program) {
    if (auto mp = std::get_if<bf::ModifyPointer>(&instruction)) {
      currentOffset += mp->offset;
      continue;
    }

    bool barrier = false;

    std::visit(
        overload{[&](bf::ModifyValue& mv) { mv.offset += currentOffset; },
                 [&](bf::ReadChar& rc) { rc.offset += currentOffset; },
                 [&](bf::WriteChar& wc) { wc.offset += currentOffset; },
                 [&](bf::LoopStart&) { barrier = true; },
                 [&](bf::LoopEnd&) { barrier = true; },
                 [&](auto&&) { Assert(false); }},
        instruction);

    if (barrier) {
      if (currentOffset != 0) {
        optimizedProgram.emplace_back(bf::ModifyPointer{currentOffset});
      }

      currentOffset = 0;
    }

    optimizedProgram.emplace_back(instruction);
  }

  return optimizedProgram;
}

const std::string& bf::passes::ReducePointerMovesPass::GetPassName() const {
  static std::string passName = "Reduce pointer moves pass";

  return passName;
}
