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
struct AssignIndexExpr { ExprPtr object; ExprPtr index; ExprPtr value; };
enum class UnaryOperator { Negate, Not, BitNot };
struct UnaryExpr { UnaryOperator op; ExprPtr operand; };
enum class BinaryOperator {
    Add, Subtract, Multiply, Divide, FloorDivide, Modulo,
    Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual,
    LogicalAnd, LogicalOr, BitwiseAnd, BitwiseOr, BitwiseXor, ShiftLeft, ShiftRight
};
struct BinaryExpr { BinaryOperator op; ExprPtr left; ExprPtr right; };
struct CompoundAssignIndexExpr { ExprPtr object; ExprPtr index; ExprPtr value; BinaryOperator op; };
struct TernaryExpr { ExprPtr condition; ExprPtr when_true; ExprPtr when_false; };
struct CallExpr { ExprPtr callee; std::vector<ExprPtr> arguments; };
struct GetPropertyExpr { ExprPtr object; std::string name; };
struct SetPropertyExpr { ExprPtr object; std::string name; ExprPtr value; };
struct CastExpr { ExprPtr value; std::string type_name; };
struct LambdaExpr { std::vector<std::string> parameters; std::vector<StmtPtr> body; };
struct IndexExpr { ExprPtr object; ExprPtr index; };

struct Expr {
    using Node = std::variant<LiteralExpr, VariableExpr, AssignExpr, UnaryExpr, BinaryExpr,
                              TernaryExpr, CallExpr, GetPropertyExpr, SetPropertyExpr, CastExpr, LambdaExpr,
                              IndexExpr, AssignIndexExpr, CompoundAssignIndexExpr>;
    SourceSpan span;
    Node node;

    template <typename T>
    Expr(SourceSpan source_span, T value) : span(std::move(source_span)), node(std::move(value)) {}
};

struct ExpressionStmt { ExprPtr expression; };
struct VariableDeclStmt { std::string name; ExprPtr initializer; std::string fixed_type; bool is_constant = false; };
struct MakeConstantStmt { std::string name; };
struct BlockStmt { std::vector<StmtPtr> statements; };
struct IfStmt { ExprPtr condition; StmtPtr then_branch; StmtPtr else_branch; };
struct WhileStmt { ExprPtr condition; StmtPtr body; };
struct ForEachStmt { std::vector<std::string> names; ExprPtr iterable; StmtPtr body; };
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
struct BreakStmt {};
struct ContinueStmt {};
struct ThrowStmt { std::string error_kind; };
struct TryStmt {
    StmtPtr body;
    std::optional<std::string> error_name;
    StmtPtr catch_body;
    StmtPtr finally_body;
};
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
                              WhileStmt, ForEachStmt, ReflexStmt, FunctionDeclStmt, ReturnStmt, MakeConstantStmt,
                              ClassDeclStmt, ImportStmt, ExportStmt, BreakStmt, ContinueStmt,
                              ThrowStmt, TryStmt>;
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
