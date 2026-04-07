#pragma once

#include <cstdint>
#include <string>

enum class PPValueKind : uint8_t {
  IMMEDIATE = 1, REFERENCE
};

struct PPValue {
  PPValueKind kind;
  std::string value;
};