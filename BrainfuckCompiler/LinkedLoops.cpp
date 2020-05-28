#include "LinkedLoops.hpp"
#include "VisitorHelper.hpp"

std::optional<size_t> bf::LinkedLoops::TryGet(const LoopMap& map, size_t key) {
  const auto iter = map.find(key);
  if (iter != map.end()) {
    return iter->second;
  }

  return std::nullopt;
}

std::optional<bf::LinkedLoops> bf::LinkedLoops::Link(const Program& program) {
  std::vector<size_t> loopStack;

  LoopMap beginToEnd;
  LoopMap endToBegin;

  for (size_t i = 0; i < program.size(); ++i) {
    const auto& instruction = program[i];

    auto unmatched = false;

    std::visit(
        overload{[&](const LoopStart& _ls) { loopStack.emplace_back(i); },
                 [&](const LoopEnd& _le) {
                   if (loopStack.empty()) {
                     unmatched = true;
                     return;
                   }

                   const auto begin = loopStack.back();
                   const auto end = i;

                   beginToEnd[begin] = end;
                   endToBegin[end] = begin;

                   loopStack.pop_back();
                 },
                 [](auto&&) {}},
        instruction);

    if (unmatched) {
      return std::nullopt;
    }
  }

  if (!loopStack.empty()) {
    return std::nullopt;
  }

  return LinkedLoops(beginToEnd, endToBegin);
}
