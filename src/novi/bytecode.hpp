#pragma once

#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "diagnostic.hpp"

namespace novi {

struct FunctionPrototype;

enum class OpCode {
    Constant, Null, True, False, Pop,
    GetName, DefineName, SetName,
    Negate, Not, Add, Subtract, Multiply, Divide,
    Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual,
    Jump, JumpIfFalse, EnterScope, ExitScope,
    MakeClosure, DefineClass, GetProperty, SetProperty,
    Call, Return
};

using Constant = std::variant<std::monostate, bool, double, std::string>;

struct Instruction {
    OpCode op;
    int operand = 0;
    SourceSpan span;
};

struct ClassPrototype {
    std::string name;
    std::vector<std::pair<std::string, std::shared_ptr<FunctionPrototype>>> methods;
};

struct Chunk {
    std::vector<Instruction> code;
    std::vector<Constant> constants;
    std::vector<std::shared_ptr<FunctionPrototype>> functions;
    std::vector<ClassPrototype> classes;
};

struct FunctionPrototype {
    std::string name;
    std::vector<std::string> parameters;
    Chunk chunk;
    SourceSpan declaration_span;
};

struct CompiledModule {
    std::shared_ptr<FunctionPrototype> entry;
    Diagnostics diagnostics;
};

}  // namespace novi
