#pragma once

#include <memory>

#include "cxxtype.h"

struct TypeRegistry {
    // Dummy impl
    TypePtr getPrimitive() {
        return std::make_shared<const PrimitiveType>(PrimitiveKind::BOOL);
    }
};