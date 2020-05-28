#pragma once
#include <istream>
#include "Instruction.hpp"

namespace bf {

using Program = std::vector<Instruction>;

Program ParseProgram(std::istream& input);

}  // namespace bf