#include "Compiler.hpp"
#include <Windows.h>
#include "../Assert.hpp"
#include "../VisitorHelper.hpp"
#include "Codegen.hpp"

void bf::jit::Compiler::CompileInstruction(const Instruction& instruction) {
  std::visit(
      overload{[&](const ModifyPointer& mp) {
                 codegen::EmitModifyPointer(mp.offset, compiledCode);
               },
               [&](const ModifyValue& mv) {
                 codegen::EmitModifyValue(mv.offset, mv.difference,
                                          compiledCode);
               },
               [&](const WriteChar& mv) {
                 codegen::EmitWriteChar(mv.offset, compiledCode);
               },
               [&](const ReadChar& rc) {
                 codegen::EmitReadChar(rc.offset, compiledCode);
               },
               [&](const LoopStart& ls) {
                 loopStack.emplace_back(codegen::EmitLoopStart(compiledCode));
               },
               [&](const LoopEnd& le) {
                 Assert(!loopStack.empty());

                 const auto instance = loopStack.back();
                 loopStack.pop_back();

                 codegen::EmitLoopEnd(instance, compiledCode);
               },
               [&](auto&&) { Assert(false); }},
      instruction);
}

void bf::jit::Compiler::Compile() {
  Assert(compiledCode.GetSize() == 0);

  codegen::EmitPrologue(compiledCode);

  for (const auto& instruction : program) {
    CompileInstruction(instruction);
  }

  codegen::EmitEpilogue(compiledCode);
}

void bf::jit::Compiler::Run(size_t bufferSize) {
  Assert(compiledCode.GetSize() > 0);

  const auto& underlying = compiledCode.Underlying();

  const auto memory =
      VirtualAlloc(nullptr, underlying.size(), MEM_COMMIT | MEM_RESERVE,
                   PAGE_EXECUTE_READWRITE);

  if (memory) {
    std::memcpy(memory, underlying.data(), underlying.size());

    std::vector<uint8_t> buffer(bufferSize);

    using JITEntrypoint = void (*)(void*, void*, void*);

    const auto entry = JITEntrypoint(memory);

    entry(buffer.data(), putchar, getchar);

    VirtualFree(memory, 0, MEM_FREE);
  }
}