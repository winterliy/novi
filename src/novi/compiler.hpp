#pragma once

#include <memory>

#include "ast.hpp"
#include "bytecode.hpp"

namespace novi {

class Compiler {
public:
    CompiledModule compile(const Program& program);
};

}  // namespace novi
