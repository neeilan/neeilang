#pragma once

#include <cactus-table.h>

struct Stmt;

using DeclCtx = CactusTable<std::string, Stmt const*>;