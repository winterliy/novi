#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "source.hpp"

namespace novi {

struct Expr;
struct Stmt;
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

struct LiteralExpr { std::variant<std::monostate, bool, double, std::string> value; };
struct VariableExpr { std::string name; };
struct AssignExpr { std::string name; ExprPtr value; };
enum class UnaryOperator { Negate, Not, BitNot };
struct UnaryExpr { UnaryOperator op; ExprPtr operand; };
enum class BinaryOperator {
    Add, Subtract, Multiply, Divide, FloorDivide, Modulo,
    Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual,
    LogicalAnd, LogicalOr, BitwiseAnd, BitwiseOr, BitwiseXor, ShiftLeft, ShiftRight
};
struct BinaryExpr { BinaryOperator op; ExprPtr left; ExprPtr right; };
struct TernaryExpr { ExprPtr condition; ExprPtr when_true; ExprPtr when_false; };
struct CallExpr { ExprPtr callee; std::vector<ExprPtr> arguments; };
struct GetPropertyExpr { ExprPtr object; std::string name; };
struct SetPropertyExpr { ExprPtr object; std::string name; ExprPtr value; };
struct LambdaExpr { std::vector<std::string> parameters; std::vector<StmtPtr> body; };

struct Expr {
    using Node = std::variant<LiteralExpr, VariableExpr, AssignExpr, UnaryExpr, BinaryExpr,
                              TernaryExpr, CallExpr, GetPropertyExpr, SetPropertyExpr, LambdaExpr>;
    SourceSpan span;
    Node node;

    template <typename T>
    Expr(SourceSpan source_span, T value) : span(std::move(source_span)), node(std::move(value)) {}
};

struct ExpressionStmt { ExprPtr expression; };
struct VariableDeclStmt { std::string name; ExprPtr initializer; std::string fixed_type; bool is_constant = false; };
struct BlockStmt { std::vector<StmtPtr> statements; };
struct IfStmt { ExprPtr condition; StmtPtr then_branch; StmtPtr else_branch; };
struct WhileStmt { ExprPtr condition; StmtPtr body; };
struct ReflexStmt { ExprPtr value; std::string target; };
struct FunctionDeclStmt {
    std::string name;
    std::vector<std::string> parameters;
    std::vector<StmtPtr> body;
    bool is_async = false;
    bool is_static = false;
    bool is_private = false;
    std::vector<std::string> attributes;
};
struct ReturnStmt { ExprPtr value; };
struct FieldDeclStmt { std::string name; ExprPtr initializer; bool is_static = false; bool is_private = false; };
struct ClassDeclStmt {
    std::string name;
    std::optional<std::string> superclass;
    std::vector<FunctionDeclStmt> methods;
    std::vector<FieldDeclStmt> fields;
};
struct ImportStmt { std::string module; std::vector<std::pair<std::string, std::string>> names; };
struct ExportStmt { std::vector<std::string> names; std::string alias; };

struct Stmt {
    using Node = std::variant<ExpressionStmt, VariableDeclStmt, BlockStmt, IfStmt,
                              WhileStmt, ReflexStmt, FunctionDeclStmt, ReturnStmt,
                              ClassDeclStmt, ImportStmt, ExportStmt>;
    SourceSpan span;
    Node node;

    template <typename T>
    Stmt(SourceSpan source_span, T value) : span(std::move(source_span)), node(std::move(value)) {}
};

struct Program {
    std::vector<StmtPtr> statements;
    bool echo_last_expression = false;  // REPL behavior, not Novi source syntax.
};

}  // namespace novi
