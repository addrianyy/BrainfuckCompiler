#include "ProgramInterpreter.hpp"
#include "Assert.hpp"
#include "LinkedLoops.hpp"
#include "VisitorHelper.hpp"

void bf::interpreter::RunProgramInterpreted(const Program& program,
                                            size_t bufferSize) {
  std::vector<uint8_t> buffer(bufferSize);

  uint8_t* currentPointer = buffer.data();
  size_t instructionIndex = 0;

  const auto loops = LinkedLoops::Link(program);

  while (instructionIndex < program.size()) {
    bool advanceII = true;

    std::visit(
        overload{[&](const ModifyPointer& mp) { currentPointer += mp.offset; },
                 [&](const ModifyValue& mv) {
                   *(currentPointer + mv.offset) += mv.difference;
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
                   std::putchar(char(*(currentPointer + wc.offset)));
                 },
                 [&](const ReadChar& rc) {
                   *(currentPointer + rc.offset) = uint8_t(std::getchar());
                 },
                 [&](auto&&) { Assert(false); }},
        program[instructionIndex]);

    if (advanceII) {
      instructionIndex += 1;
    }
  }
}
