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
    Constant, Null, True, False, Pop, Duplicate, DuplicatePair, ToBoolean,
    GetName, DefineName, DefineConstantName, DefineTypedName, DefineTypedConstantName,
    FreezeName, SetName, Cast,
    Negate, Not, BitNot, Add, Subtract, Multiply, Divide, FloorDivide, Modulo,
    BitwiseAnd, BitwiseOr, BitwiseXor, ShiftLeft, ShiftRight, LogicalAnd, LogicalOr,
    Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual,
    Jump, JumpIfFalse, EnterScope, ExitScope, IteratorInit, IteratorNext,
    PushExceptionHandler, PopExceptionHandler, Throw, Rethrow,
    GetIndex, SetIndex,
    MakeClosure, DefineClass, GetProperty, SetProperty,
    Call, Return
};

using Constant = std::variant<std::monostate, bool, double, std::string>;

struct Instruction {
    OpCode op;
    int operand = 0;
    SourceSpan span;
    int operand2 = 0;
    int operand3 = 0;
    int operand4 = 0;
};

struct ClassPrototype {
    std::string name;
    std::vector<std::pair<std::string, std::shared_ptr<FunctionPrototype>>> methods;
    std::vector<std::pair<std::string, std::string>> field_types;
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
