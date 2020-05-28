#include "ReducePointerMovesPass.hpp"
#include "../Assert.hpp"
#include "../VisitorHelper.hpp"

void bf::passes::ReducePointerMovesPass::Initialize() {}

bf::Program bf::passes::ReducePointerMovesPass::Optimize(bf::Program program) {
  using namespace instrs;

  bf::Program optimizedProgram;
  optimizedProgram.reserve(program.size());

  int64_t currentOffset = 0;

  for (auto& instruction : program) {
    if (const auto mp = std::get_if<ModifyPointer>(&instruction)) {
      currentOffset += mp->offset;
      continue;
    }

    bool barrier = false;

    std::visit(overload{[&](ModifyValue& mv) { mv.offset += currentOffset; },
                        [&](ReadChar& rc) { rc.offset += currentOffset; },
                        [&](WriteChar& wc) { wc.offset += currentOffset; },
                        [&](SetValue& sv) { sv.offset += currentOffset; },
                        [&](CopyAddValue& cav) {
                          cav.from += currentOffset;
                          cav.to += currentOffset;
                        },
                        [&](CopyValue& cv) {
                          cv.from += currentOffset;
                          cv.to += currentOffset;
                        },
                        [&](LoopStart&) { barrier = true; },
                        [&](LoopEnd&) { barrier = true; },
                        [&](auto&&) { Assert(false); }},
               instruction);

    if (barrier) {
      if (currentOffset != 0) {
        optimizedProgram.emplace_back(ModifyPointer{currentOffset});
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
