#pragma once
#include <vector>

namespace bf::jit {

class ByteBuffer {
  std::vector<uint8_t> buffer;

 public:
  size_t GetSize() const { return buffer.size(); }
  std::vector<uint8_t>& Underlying() { return buffer; }

  template <typename T>
  size_t Push(T value) {
    const auto size = buffer.size();

    buffer.reserve(buffer.size() + sizeof(T));

    uint8_t bytes[sizeof(T)];
    std::memcpy(bytes, &value, sizeof(T));

    for (const auto byte : bytes) {
      buffer.emplace_back(byte);
    }

    return size;
  }

  size_t PushBytes(const std::initializer_list<uint8_t>& bytes) {
    const auto size = buffer.size();

    buffer.reserve(buffer.size() + bytes.size());

    for (const auto byte : bytes) {
      buffer.emplace_back(byte);
    }

    return size;
  }
};

}  // namespace bf::jit