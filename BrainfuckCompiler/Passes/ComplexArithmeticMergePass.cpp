#include "ComplexArithmeticMergePass.hpp"
#include <unordered_map>
#include "../Assert.hpp"

static bool IsLoop(const bf::Instruction& instruction) {
  return std::holds_alternative<bf::instrs::LoopStart>(instruction) ||
         std::holds_alternative<bf::instrs::LoopEnd>(instruction);
}

void bf::passes::ComplexArithmeticMergePass::Initialize() {}

bf::Program bf::passes::ComplexArithmeticMergePass::Optimize(
    bf::Program program) {
  using namespace instrs;

  bf::Program optimizedProgram;
  optimizedProgram.reserve(program.size());

  struct ValueChange {
    int64_t value = 0;
    bool absolute = false;
  };

  std::unordered_map<int64_t, ValueChange> mapping;

  for (size_t i = 0; i < program.size(); ++i) {
    if (IsLoop(program[i])) {
      optimizedProgram.emplace_back(program[i]);
      continue;
    }

    mapping.clear();

    const auto applyVC = [&](int64_t offset, const ValueChange& change) {
      if (change.absolute) {
        optimizedProgram.emplace_back(SetValue{offset, uint8_t(change.value)});
        return;
      }

      if (change.value != 0) {
        optimizedProgram.emplace_back(ModifyValue{offset, change.value});
      }
    };

    const auto applyAllVCs = [&]() {
      for (const auto& [offset, change] : mapping) {
        applyVC(offset, change);
      }
    };

    const auto applyVCAtOffset = [&](int64_t offset) {
      const auto it = mapping.find(offset);
      if (it != mapping.end()) {
        applyVC(offset, it->second);
        mapping.erase(offset);
      }
    };

    size_t j = i;

    for (; j < program.size(); ++j) {
      const auto& instruction = program[j];

      if (const auto mv = std::get_if<ModifyValue>(&instruction)) {
        mapping[mv->offset].value += mv->difference;
        continue;
      }

      if (const auto sv = std::get_if<SetValue>(&instruction)) {
        mapping[sv->offset] = ValueChange{sv->value, true};
        continue;
      }

      if (const auto cv = std::get_if<CopyValue>(&instruction)) {
        const auto fromIt = mapping.find(cv->from);
        if (fromIt != mapping.end() && fromIt->second.absolute) {
          const auto newValue = fromIt->second.value;

          mapping[cv->to] = ValueChange{newValue, true};

          continue;
        }

        applyVCAtOffset(cv->from);
        mapping.erase(cv->to);

        optimizedProgram.emplace_back(instruction);

        continue;
      }

      if (const auto cav = std::get_if<CopyAddValue>(&instruction)) {
        const auto fromIt = mapping.find(cav->from);
        if (fromIt != mapping.end() && fromIt->second.absolute) {
          const auto addValue = fromIt->second.value * cav->mult;

          mapping[cav->to].value += addValue;

          continue;
        }

        const auto toIt = mapping.find(cav->to);
        if (toIt != mapping.end() && toIt->second.absolute &&
            toIt->second.value == 0 && cav->mult == 1) {
          applyVCAtOffset(cav->from);
          mapping.erase(cav->to);

          optimizedProgram.emplace_back(CopyValue{cav->from, cav->to});

          continue;
        }

        applyVCAtOffset(cav->from);
        applyVCAtOffset(cav->to);

        optimizedProgram.emplace_back(instruction);

        continue;
      }

      const auto rc = std::get_if<ReadChar>(&instruction);
      const auto wc = std::get_if<WriteChar>(&instruction);
      if (rc || wc) {
        const auto offset = rc ? rc->offset : wc->offset;

        applyVCAtOffset(offset);

        optimizedProgram.emplace_back(instruction);

        continue;
      }

      if (IsLoop(instruction) ||
          std::holds_alternative<ModifyPointer>(instruction)) {
        applyAllVCs();
        optimizedProgram.emplace_back(instruction);

        break;
      }

      Assert(false && "Unknown instruction.");
    }

    if (j == program.size()) {
      applyAllVCs();
    }

    i = j;
  }

  return optimizedProgram;
}

const std::string& bf::passes::ComplexArithmeticMergePass::GetPassName() const {
  static std::string passName = "Complex arithmetic merge pass";

  return passName;
}
