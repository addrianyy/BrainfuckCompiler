#include "Codegen.hpp"

// Register allocation:
// RBX - current element pointer
// R12 - void put_char(char)
// R13 - char get_char()

constexpr size_t JmpSize = 5;
constexpr size_t JzSize = 6;

static uint32_t CalculateRel32(uint64_t current,
                               uint64_t target,
                               uint64_t instructionLength) {
  return uint32_t(target - current - instructionLength);
}

void bf::jit::codegen::EmitPrologue(ByteBuffer& buffer) {
  /*
  push rbx
  push r12
  push r13
  sub rsp, 0x20

  mov rbx, rcx
  mov r12, rdx
  mov r13, r8
  */
  buffer.PushBytes({0x53, 0x41, 0x54, 0x41, 0x55, 0x48, 0x83, 0xEC, 0x20, 0x48,
                    0x89, 0xCB, 0x49, 0x89, 0xD4, 0x4D, 0x89, 0xC5});
}

void bf::jit::codegen::EmitEpilogue(ByteBuffer& buffer) {
  /*
  add rsp, 0x20
  pop r13
  pop r12
  pop rbx
  ret
  */
  buffer.PushBytes(
      {0x48, 0x83, 0xC4, 0x20, 0x41, 0x5D, 0x41, 0x5C, 0x5B, 0xC3});
}

void bf::jit::codegen::EmitModifyPointer(int64_t offset, ByteBuffer& buffer) {
  if (offset == 0) {
    return;
  }

  // add rbx, imm32
  buffer.PushBytes({0x48, 0x81, 0xC3});
  buffer.Push(uint32_t(offset));
}

void bf::jit::codegen::EmitModifyValue(int64_t offset,
                                       int64_t difference,
                                       ByteBuffer& buffer) {
  difference = int64_t(int8_t(difference & 0xFF));

  if (difference == 0) {
    return;
  }

  // add byte ptr [rbx+imm32], imm8
  buffer.PushBytes({0x80, 0x83});
  buffer.Push(uint32_t(offset));
  buffer.Push(uint8_t(difference));
}

void bf::jit::codegen::EmitWriteChar(int64_t offset, ByteBuffer& buffer) {
  // mov cl, byte ptr [rbx+imm32]
  buffer.PushBytes({0x8A, 0x8B});
  buffer.Push(uint32_t(offset));

  // call r12
  buffer.PushBytes({0x41, 0xFF, 0xD4});
}

void bf::jit::codegen::EmitReadChar(int64_t offset, ByteBuffer& buffer) {
  // call r13
  buffer.PushBytes({0x41, 0xFF, 0xD5});

  // mov byte ptr [rbx+imm32], al
  buffer.PushBytes({0x88, 0x83});
  buffer.Push(uint32_t(offset));
}

bf::jit::LoopInstance bf::jit::codegen::EmitLoopStart(ByteBuffer& buffer) {
  // cmp byte ptr [rbx], 0
  const auto condition = buffer.PushBytes({0x80, 0x3B, 0x00});

  // jz rel32
  const auto jumpToTheEnd =
      buffer.PushBytes({0x0F, 0x84, 0x00, 0x00, 0x00, 0x00});

  return {condition, jumpToTheEnd};
}

void bf::jit::codegen::EmitLoopEnd(const LoopInstance& instance,
                                   ByteBuffer& buffer) {
  const auto currentIP = buffer.GetSize();
  const auto targetIP = instance.condition;

  // jmp rel32 (loop condition)
  buffer.PushBytes({0xE9});
  buffer.Push(CalculateRel32(currentIP, targetIP, JmpSize));

  const auto afterLoop = buffer.GetSize();
  const auto failedJumpRel32 =
      CalculateRel32(instance.jumpToTheEnd, afterLoop, JzSize);

  // jz rel32 (end of loop)
  std::memcpy(buffer.Underlying().data() + instance.jumpToTheEnd + JzSize - 4,
              &failedJumpRel32, sizeof(failedJumpRel32));
}
