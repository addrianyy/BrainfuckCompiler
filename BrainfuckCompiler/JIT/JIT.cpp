#include "JIT.hpp"
#include <Windows.h>
#include <chrono>
#include <fstream>
#include <iostream>

#include "../Assert.hpp"
#include "../VisitorHelper.hpp"

static bool FitsInI8(int64_t value) {
  return value >= -127 && value <= 127;
}

static void EmitModifyPointer(int64_t offset, bf::jit::ByteBuffer& buffer) {
  if (offset == 0) {
    return;
  }

  if (offset == 1 || offset == -1) {
    const uint8_t last = offset == 1 ? 0xC3 : 0xCB;

    // inc/dec rbx
    buffer.PushBytes({0x48, 0xFF, last});
    return;
  }

  if (FitsInI8(offset)) {
    // add rbx, imm8
    buffer.PushBytes({0x48, 0x83, 0xC3});
    buffer.Push(uint8_t(offset));
    return;
  }

  // add rbx, imm32
  buffer.PushBytes({0x48, 0x81, 0xC3});
  buffer.Push(uint32_t(offset));
}

static void EmitModifyValue(int64_t offset,
                            int64_t difference,
                            bf::jit::ByteBuffer& buffer) {
  difference = int64_t(int8_t(difference & 0xFF));

  if (difference == 0) {
    return;
  }

  if (offset == 0) {
    /*
    if (difference == 1 || difference == -1) {
      const uint8_t last = difference == 1 ? 0x03 : 0x0B;

      // inc/dec byte ptr [rbx]
      buffer.PushBytes({0xFE, last});
      return;
    }
    */

    // add byte ptr [rbx], imm8
    buffer.PushBytes({0x80, 0x03});
    buffer.Push(uint8_t(difference));
    return;
  }

  /*
  if (difference == 1 || difference == -1) {
    const uint8_t last = difference == 1 ? 0x83 : 0x8B;

    // inc/dec byte ptr [rbx+imm32]
    buffer.PushBytes({0xFE, last});
    buffer.Push(uint32_t(offset));
    return;
  }
  */

  // add byte ptr [rbx+imm32], imm8
  buffer.PushBytes({0x80, 0x83});
  buffer.Push(uint32_t(offset));
  buffer.Push(uint8_t(difference));
}

static void EmitValueToCl(int64_t offset, bf::jit::ByteBuffer& buffer) {
  /*
  if (offset == 0) {
    // mov cl, byte ptr [rbx]
    buffer.PushBytes({0x8A, 0x0B});
    return;
  }

  if (FitsInI8(offset)) {
    // mov cl, byte ptr [rbx+imm8]
    buffer.PushBytes({0x8A, 0x4B});
    buffer.Push(uint8_t(offset));
    return;
  }
  */

  // mov cl, byte ptr [rbx+imm32]
  buffer.PushBytes({0x8A, 0x8B});
  buffer.Push(uint32_t(offset));
}

static void EmitWriteChar(int64_t offset, bf::jit::ByteBuffer& buffer) {
  EmitValueToCl(offset, buffer);

  // call r12
  buffer.PushBytes({0x41, 0xFF, 0xD4});
}

void bf::jit::JITCompiler::CompileInstruction(const Instruction& instruction) {
  std::visit(
      overload{[&](const ModifyPointer& mp) {
                 EmitModifyPointer(mp.offset, compiledCode);
               },
               [&](const ModifyValue& mv) {
                 EmitModifyValue(mv.offset, mv.difference, compiledCode);
               },
               [&](const WriteChar& mv) {
                 //EmitWriteChar(mv.offset, compiledCode);
                  compiledCode.PushBytes({0x8A, 0x8B});
                  compiledCode.Push(uint32_t(mv.offset));
                  compiledCode.PushBytes({0x41, 0xFF, 0xD4});
               },
               [&](const LoopStart& ls) {
                 const auto head = compiledCode.PushBytes({0x80, 0x3B, 0x00});
                 const auto jz = compiledCode.PushBytes(
                     {0x0F, 0x84, 0x00, 0x00, 0x00, 0x00});

                 loopStack.emplace_back(LoopInstance{head, jz});
               },
               [&](const LoopEnd& le) {
                 Assert(!loopStack.empty());

                 const auto loop = loopStack.back();
                 loopStack.pop_back();

                 const auto currentIP = compiledCode.Underlying().size();
                 const auto jumpBackRel32 =
                     int32_t(loop.head) - int32_t(currentIP) - 5;

                 compiledCode.PushBytes({0xE9});
                 compiledCode.Push(uint32_t(jumpBackRel32));

                 const auto jzRel32 =
                     int32_t(compiledCode.Underlying().size()) -
                     int32_t(loop.jz) - 6;

                 *(uint32_t*)(compiledCode.Underlying().data() + loop.jz + 2) =
                     jzRel32;
               },
               [&](auto&&) { Assert(false); }},
      instruction);
}

void bf::jit::JITCompiler::Compile() {
  /*
  push rbx
  push r12
  push r13
  sub rsp, 0x20

  mov rbx, rcx
  mov r12, rdx
  mov r13, r8
  */
  compiledCode.PushBytes({0x53, 0x41, 0x54, 0x41, 0x55, 0x48, 0x83, 0xEC, 0x20,
                          0x48, 0x89, 0xCB, 0x49, 0x89, 0xD4, 0x4D, 0x89,
                          0xC5});

  for (const auto& instruction : program) {
    CompileInstruction(instruction);
  }

  /*
  add rsp, 0x20
  pop r13
  pop r12
  pop rbx
  ret
  */
  compiledCode.PushBytes(
      {0x48, 0x83, 0xC4, 0x20, 0x41, 0x5D, 0x41, 0x5C, 0x5B, 0xC3});

  std::ofstream out("out.bin", std::ios::binary);

  out.write((const char*)compiledCode.Underlying().data(),
            compiledCode.Underlying().size());
}

void bf::jit::JITCompiler::Run() {
  const auto& underlying = compiledCode.Underlying();

  const auto memory =
      VirtualAlloc(nullptr, underlying.size(), MEM_COMMIT | MEM_RESERVE,
                   PAGE_EXECUTE_READWRITE);

  if (memory) {
    std::memcpy(memory, underlying.data(), underlying.size());

    using JITEntrypoint = void (*)(void*, void*, void*);

    std::vector<uint8_t> buffer(300'000);

    const auto entry = JITEntrypoint(memory);

    const auto before = std::chrono::high_resolution_clock::now();

    entry(buffer.data(), putchar, getchar);

    const auto after = std::chrono::high_resolution_clock::now();

    const auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(after - before)
            .count();
    std::cout << ms << "ms\n";

    VirtualFree(memory, 0, MEM_FREE);
  }
}
