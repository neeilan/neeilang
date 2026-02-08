#pragma once

#include "ir-instr.h"

class Interpreter {
    std::vector<const char> stack_;
    std::vector<const char> heap_;
    std::vector<uint64_t> registers_;
};
