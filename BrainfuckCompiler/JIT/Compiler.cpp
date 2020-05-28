#include "Compiler.hpp"
#include <Windows.h>
#include "../Assert.hpp"
#include "../IO.hpp"
#include "../VisitorHelper.hpp"
#include "Codegen.hpp"

void bf::jit::Compiler::CompileInstruction(const Instruction& instruction) {
  using namespace instrs;

  std::visit(
      overload{
          [&](const ModifyPointer& mp) {
            codegen::EmitModifyPointer(mp.offset, compiledCode);
          },
          [&](const ModifyValue& mv) {
            codegen::EmitModifyValue(mv.offset, mv.difference, compiledCode);
          },
          [&](const SetValue& sv) {
            codegen::EmitSetValue(sv.offset, sv.value, compiledCode);
          },
          [&](const CopyAddValue& cav) {
            codegen::EmitCopyAddValue(cav.from, cav.to, cav.mult, compiledCode);
          },
          [&](const CopyValue& cv) {
            codegen::EmitCopyValue(cv.from, cv.to, compiledCode);
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
            Assert(!loopStack.empty() && "Unmatched loop markers.");

            const auto instance = loopStack.back();
            loopStack.pop_back();

            codegen::EmitLoopEnd(instance, compiledCode);
          },
          [&](auto&&) { Assert(false && "Unknown instruction."); }},
      instruction);
}

void bf::jit::Compiler::Compile() {
  compiledCode.Underlying().reserve(program.size() * 10);

  Assert(compiledCode.GetSize() == 0 && "Code is already compiled.");

  codegen::EmitPrologue(compiledCode);

  for (const auto& instruction : program) {
    CompileInstruction(instruction);
  }

  codegen::EmitEpilogue(compiledCode);

  compiledCode.Underlying().shrink_to_fit();
}

void bf::jit::Compiler::Run(size_t bufferSize) {
  Assert(sizeof(void*) == 8 && "Only 64bit mode JIT is supported.");

  Assert(compiledCode.GetSize() > 0 && "Tried to run not compiled code.");

  const auto& underlying = compiledCode.Underlying();

  const auto memory =
      VirtualAlloc(nullptr, underlying.size(), MEM_COMMIT | MEM_RESERVE,
                   PAGE_EXECUTE_READWRITE);

  Assert(memory && "Failed to allocate RWX memory for JIT.");

  std::memcpy(memory, underlying.data(), underlying.size());

  std::vector<uint8_t> buffer(bufferSize);

  using JITEntrypoint = void (*)(void*, void*, void*);

  const auto entry = JITEntrypoint(memory);

  entry(buffer.data(), (void*)bf::io::WriteChar, (void*)bf::io::ReadChar);

  VirtualFree(memory, 0, MEM_FREE);
}
