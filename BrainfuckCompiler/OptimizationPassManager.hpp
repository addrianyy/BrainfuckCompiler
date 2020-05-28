#pragma once
#include <memory>
#include "OptimizationPass.hpp"
#include "Program.hpp"

namespace bf::opt {

class OptimizationPassManager {
  std::vector<std::unique_ptr<OptimizationPass>> passes;

 public:
  OptimizationPassManager() = default;

  template <typename T, typename... Args>
  void Add(Args&&... args) {
    passes.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
  }

  bool RunOnProgram(bf::Program& program, bool debugLog = false);
};

}  // namespace bf::opt