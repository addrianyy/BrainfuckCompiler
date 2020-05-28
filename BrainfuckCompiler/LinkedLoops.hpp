#pragma once
#include <optional>
#include <unordered_map>
#include "Program.hpp"

namespace bf {

class LinkedLoops {
  using LoopMap = std::unordered_map<size_t, size_t>;

  LoopMap beginToEnd;
  LoopMap endToBegin;

  static std::optional<size_t> TryGet(const LoopMap& map, size_t key);

  LinkedLoops(LoopMap beginToEnd, LoopMap endToBegin)
      : beginToEnd(std::move(beginToEnd)), endToBegin(std::move(endToBegin)) {}

 public:
  static std::optional<LinkedLoops> Link(const Program& program);

  std::optional<size_t> EndFromBegin(size_t begin) const {
    return TryGet(beginToEnd, begin);
  }

  std::optional<size_t> BeginFromEnd(size_t end) const {
    return TryGet(endToBegin, end);
  }
};

}  // namespace bf