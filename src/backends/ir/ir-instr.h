#pragma once

#define IRINSTR_LIST(X) \
    X(PRINT)            \
    X(ADD)              \
    X(SUB)              \
    X(CALL)             \
    X(RET)              \
    X(SAL)              \
    X(SAR)              \
    X(LOAD)             \
    X(STORE)            \
    X(MEMLOAD)          \
    X(MEMSTORE)

    enum class IRInstr {
#define X(name) name,
    IRINSTR_LIST(X)
#undef X
};

inline const char* str(IRInstr e) {
    switch (e) {
#define X(name) case IRInstr::name: return #name;
        IRINSTR_LIST(X)
#undef X
    default:
        return "<unknown>";
    }
}

struct Label {};
struct RegisterRef { uint32_t id; };