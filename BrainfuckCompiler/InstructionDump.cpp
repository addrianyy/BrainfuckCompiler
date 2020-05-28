#include "InstructionDump.hpp"
#include "Assert.hpp"
#include "Instruction.hpp"
#include "LinkedLoops.hpp"
#include "VisitorHelper.hpp"

static void DumpModifyPointer(std::ostream& output,
                              const bf::instrs::ModifyPointer& mp) {
  auto offset = mp.offset;

  if (mp.offset < 0) {
    output << "p -= ";
    offset = -offset;
  } else {
    output << "p += ";
  }

  output << offset << "\n";
}

static void DumpModifyValue(std::ostream& output,
                            const bf::instrs::ModifyValue& mv) {
  auto difference = mv.difference;

  output << "p[" << mv.offset << "] ";

  if (mv.difference < 0) {
    output << "-= ";
    difference = -difference;
  } else {
    output << "+= ";
  }

  output << difference << "\n";
}

static void DumpWriteChar(std::ostream& output,
                          const bf::instrs::WriteChar& wc) {
  output << "putchar(p[" << wc.offset << "])\n";
}

static void DumpReadChar(std::ostream& output, const bf::instrs::ReadChar& rc) {
  output << "p[" << rc.offset << "] = getchar()\n";
}

static void DumpSetValue(std::ostream& output, const bf::instrs::SetValue& sv) {
  output << "p[" << sv.offset << "] = " << uint32_t(sv.value) << "\n";
}

static void DumpCopyAddValue(std::ostream& output,
                             const bf::instrs::CopyAddValue& cav) {
  output << "p[" << cav.to << "] += p[" << cav.from << "]";

  if (cav.mult != 1) {
    output << " * " << cav.mult;
  }

  output << "\n";
}

static void DumpCopyValue(std::ostream& output,
                          const bf::instrs::CopyValue& cv) {
  output << "p[" << cv.to << "] = p[" << cv.from << "]\n";
}

void bf::DumpInstruction(std::ostream& output, const Instruction& instruction) {
  using namespace instrs;

  std::visit(
      overload{[&](const ModifyPointer& mp) { DumpModifyPointer(output, mp); },
               [&](const ModifyValue& mv) { DumpModifyValue(output, mv); },
               [&](const WriteChar& wc) { DumpWriteChar(output, wc); },
               [&](const ReadChar& rc) { DumpReadChar(output, rc); },
               [&](const LoopStart& ls) { output << "loop_start\n"; },
               [&](const LoopEnd& ls) { output << "loop_end\n"; },
               [&](const SetValue& sv) { DumpSetValue(output, sv); },
               [&](const CopyAddValue& cav) { DumpCopyAddValue(output, cav); },
               [&](const CopyValue& cv) { DumpCopyValue(output, cv); },
               [](auto&&) { Assert(false && "Unknown instruction."); }},
      instruction);
}

void bf::DumpProgram(std::ostream& output, const Program& program) {
  const auto loops = LinkedLoops::Link(program);

  std::unordered_map<size_t, uint64_t> labelNumbers;

  uint64_t nextLabelNumber = 0;

  output << "@entry:" << '\n';

  for (size_t i = 0; i < program.size(); ++i) {
    const auto& instruction = program[i];

    if (std::holds_alternative<instrs::LoopStart>(instruction)) {
      const auto labelNumber = nextLabelNumber++;
      labelNumbers[i] = labelNumber;

      output << "@begin_" << labelNumber << ":" << '\n';
      output << "  jumpzero(p[0], @end_" << labelNumber << ")" << '\n';
      continue;
    }

    if (std::holds_alternative<instrs::LoopEnd>(instruction)) {
      const auto begin = *loops->BeginFromEnd(i);
      const auto labelNumber = labelNumbers[begin];

      output << "  jump(@begin_" << labelNumber << ")\n";
      output << "@end_" << labelNumber << ":" << '\n';
      continue;
    }

    output << "  ";
    DumpInstruction(output, instruction);
  }
}
