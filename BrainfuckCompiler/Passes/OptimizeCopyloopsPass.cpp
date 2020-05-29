#include "OptimizeCopyloopsPass.hpp"
#include <iostream>
#include <unordered_map>
#include "../InstructionDump.hpp"

void bf::passes::OptimizeCopyloopsPass::Initialize() {}

bf::Program bf::passes::OptimizeCopyloopsPass::Optimize(bf::Program program) {
  using namespace instrs;

  bf::Program optimizedProgram;
  optimizedProgram.reserve(program.size());

  std::unordered_map<int64_t, int64_t> mapping;

  for (size_t i = 0; i < program.size(); ++i) {
    if (!std::holds_alternative<LoopStart>(program[i])) {
      optimizedProgram.emplace_back(program[i]);
      continue;
    }

    mapping.clear();

    size_t j = i + 1;
    int64_t currentOffset = 0;

    bool brokeOnLoop = false;

    for (; j < program.size(); ++j) {
      const auto& instruction = program[j];
      if (std::holds_alternative<LoopEnd>(instruction)) {
        brokeOnLoop = true;
        break;
      }

      if (const auto mv = std::get_if<ModifyValue>(&instruction)) {
        mapping[currentOffset + mv->offset] += mv->difference;
        continue;
      }

      if (const auto mp = std::get_if<ModifyPointer>(&instruction)) {
        currentOffset += mp->offset;
        continue;
      }

      break;
    }

    bool folded = false;

    if (brokeOnLoop && currentOffset == 0) {
      const auto zeroIt = mapping.find(0);
      if (zeroIt != mapping.end() && zeroIt->second == -1) {
        folded = true;

        for (const auto& [offset, mult] : mapping) {
          if (offset == 0) {
            continue;
          }

          optimizedProgram.emplace_back(CopyAddValue{0, offset, mult});
        }

        optimizedProgram.emplace_back(SetValue{0, 0});
      }
    }

    if (!folded) {
      for (size_t k = i; k < j; ++k) {
        optimizedProgram.emplace_back(program[k]);
      }

      j -= 1;
    }

    i = j;
  }

  return optimizedProgram;
}

const std::string& bf::passes::OptimizeCopyloopsPass::GetPassName() const {
  static std::string passName = "Optimize copyloops pass";

  return passName;
}
