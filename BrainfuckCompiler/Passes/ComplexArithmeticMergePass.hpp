#pragma once
#include "../OptimizationPass.hpp"

namespace bf::passes {

class ComplexArithmeticMergePass : public bf::opt::OptimizationPass {
 public:
  virtual void Initialize();
  virtual bf::Program Optimize(bf::Program program);

  virtual const std::string& GetPassName() const;
};

}  // namespace bf::passes