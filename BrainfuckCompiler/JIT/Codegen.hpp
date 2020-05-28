#pragma once
#include "ByteBuffer.hpp"
#include "Compiler.hpp"

namespace bf::jit::codegen {

void EmitPrologue(ByteBuffer& buffer);
void EmitEpilogue(ByteBuffer& buffer);

void EmitModifyPointer(int64_t offset, ByteBuffer& buffer);
void EmitModifyValue(int64_t offset, int64_t difference, ByteBuffer& buffer);

void EmitWriteChar(int64_t offset, ByteBuffer& buffer);
void EmitReadChar(int64_t offset, ByteBuffer& buffer);

void EmitSetValue(int64_t offset, uint8_t value, ByteBuffer& buffer);
void EmitCopyValue(int64_t from, int64_t to, ByteBuffer& buffer);
void EmitCopyAddValue(int64_t from,
                      int64_t to,
                      int64_t mult,
                      ByteBuffer& buffer);

LoopInstance EmitLoopStart(ByteBuffer& buffer);
void EmitLoopEnd(const LoopInstance& instance, ByteBuffer& buffer);

}  // namespace bf::jit::codegen