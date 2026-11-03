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
enum class UnaryOperator { Negate, Not };
struct UnaryExpr { UnaryOperator op; ExprPtr operand; };
enum class BinaryOperator { Add, Subtract, Multiply, Divide, Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual };
struct BinaryExpr { BinaryOperator op; ExprPtr left; ExprPtr right; };
struct CallExpr { ExprPtr callee; std::vector<ExprPtr> arguments; };
struct GetPropertyExpr { ExprPtr object; std::string name; };
struct SetPropertyExpr { ExprPtr object; std::string name; ExprPtr value; };

struct Expr {
    using Node = std::variant<LiteralExpr, VariableExpr, AssignExpr, UnaryExpr, BinaryExpr,
                              CallExpr, GetPropertyExpr, SetPropertyExpr>;
    SourceSpan span;
    Node node;

    template <typename T>
    Expr(SourceSpan source_span, T value) : span(std::move(source_span)), node(std::move(value)) {}
};

struct ExpressionStmt { ExprPtr expression; };
struct VariableDeclStmt { std::string name; ExprPtr initializer; };
struct BlockStmt { std::vector<StmtPtr> statements; };
struct IfStmt { ExprPtr condition; StmtPtr then_branch; StmtPtr else_branch; };
struct WhileStmt { ExprPtr condition; StmtPtr body; };
struct FunctionDeclStmt { std::string name; std::vector<std::string> parameters; std::vector<StmtPtr> body; };
struct ReturnStmt { ExprPtr value; };
struct ClassDeclStmt {
    std::string name;
    std::optional<std::string> superclass;
    std::vector<FunctionDeclStmt> methods;
};

struct Stmt {
    using Node = std::variant<ExpressionStmt, VariableDeclStmt, BlockStmt, IfStmt,
                              WhileStmt, FunctionDeclStmt, ReturnStmt, ClassDeclStmt>;
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
