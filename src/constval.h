#pragma once

#include <string>
#include <optional>
#include <variant>
#include <vector>


using CompileTimeBuiltin = std::variant<int, bool, double, char>;

struct CompileTimePtr {};

// For simplicity, don't allow literal classes that have non-builtin fields
// to be constexpr
struct CompileTimeRecordField : public CompileTimeBuiltin {
    std::string name;
    size_t offset;
};

struct CompileTimeRecord {
    std::vector<std::byte> value;
    std::vector<CompileTimeRecordField> fields;
};

struct CompileTimeValue
    : public std::variant<CompileTimeBuiltin, CompileTimePtr, CompileTimeRecord> {};
