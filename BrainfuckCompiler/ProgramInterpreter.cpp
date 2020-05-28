#include "ProgramInterpreter.hpp"
#include "Assert.hpp"
#include "IO.hpp"
#include "LinkedLoops.hpp"
#include "VisitorHelper.hpp"

void bf::interpreter::RunProgramInterpreted(const Program& program,
                                            size_t bufferSize) {
  using namespace instrs;

  std::vector<uint8_t> buffer(bufferSize);

  uint8_t* currentPointer = buffer.data();
  size_t instructionIndex = 0;

  const auto loops = LinkedLoops::Link(program);

  while (instructionIndex < program.size()) {
    bool advanceII = true;

    std::visit(
        overload{[&](const ModifyPointer& mp) { currentPointer += mp.offset; },
                 [&](const ModifyValue& mv) {
                   *(currentPointer + mv.offset) += uint8_t(mv.difference);
                 },
                 [&](const LoopStart& ls) {
                   if (*currentPointer == 0) {
                     instructionIndex =
                         *loops->EndFromBegin(instructionIndex) + 1;
                     advanceII = false;
                   }
                 },
                 [&](const LoopEnd& le) {
                   instructionIndex = *loops->BeginFromEnd(instructionIndex);
                   advanceII = false;
                 },
                 [&](const WriteChar& wc) {
                   bf::io::WriteChar(char(*(currentPointer + wc.offset)));
                 },
                 [&](const ReadChar& rc) {
                   *(currentPointer + rc.offset) = uint8_t(bf::io::ReadChar());
                 },
                 [&](const SetValue& sv) {
                   *(currentPointer + sv.offset) = sv.value;
                 },
                 [&](const CopyAddValue& cav) {
                   *(currentPointer + cav.to) +=
                       int8_t(*(currentPointer + cav.from)) * int8_t(cav.mult);
                 },
                 [&](const CopyValue& cv) {
                   *(currentPointer + cv.to) =
                       int8_t(*(currentPointer + cv.from));
                 },
                 [&](auto&&) { Assert(false && "Unknown instruction."); }},
        program[instructionIndex]);

    if (advanceII) {
      instructionIndex += 1;
    }
  }
}
