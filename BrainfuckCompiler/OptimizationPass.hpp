#pragma once
#include "Program.hpp"

namespace bf::opt {

class OptimizationPass {
 public:
  virtual void Initialize() = 0;
  virtual bf::Program Optimize(bf::Program program) = 0;

  virtual const std::string& GetPassName() const = 0;

  virtual ~OptimizationPass() = default;
};

}  // namespace bf::opt