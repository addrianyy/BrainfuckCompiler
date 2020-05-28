#pragma once
#include <ostream>
#include "Instruction.hpp"
#include "Program.hpp"

namespace bf {

void DumpInstruction(std::ostream& output, const Instruction& instruction);
void DumpProgram(std::ostream& output, const Program& program);

}  // namespace bf