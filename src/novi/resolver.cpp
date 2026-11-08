#include "resolver.hpp"

#include <algorithm>
#include <type_traits>
#include <utility>

namespace novi {
namespace {

template <class... Ts> struct Overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;

bool is_builtin(const std::string& name) {
    static const std::unordered_set<std::string> builtins{
        "print", "mod", "istrue", "string", "integer", "range",
        "dictionary", "list", "tuple", "set", "queue"
    };
    return builtins.find(name) != builtins.end();
}

}  // namespace

Diagnostics Resolver::resolve(const Program& program) {
    scopes_.clear();
    diagnostics_.clear();
    begin_scope();
    for (const auto& statement : program.statements) resolve_statement(*statement);
    end_scope();
    return diagnostics_;
}

void Resolver::begin_scope() { scopes_.emplace_back(); }

void Resolver::end_scope() { scopes_.pop_back(); }

void Resolver::declare(const std::string& name) { scopes_.back().insert(name); }

void Resolver::require_name(const std::string& name, const SourceSpan& span) {
    if (is_builtin(name)) return;
    for (auto scope = scopes_.rbegin(); scope != scopes_.rend(); ++scope) {
        if (scope->find(name) != scope->end()) return;
    }
    diagnostics_.push_back({DiagnosticCategory::Semantic, "Undefined name '" + name + "'", span});
}

void Resolver::resolve_function(const FunctionDeclStmt& function, bool is_method) {
    begin_scope();
    if (is_method) declare("this");
    for (const auto& parameter : function.parameters) declare(parameter);
    for (const auto& statement : function.body) resolve_statement(*statement);
    end_scope();
}

void Resolver::resolve_statement(const Stmt& statement) {
    std::visit(Overloaded{
        [&](const ExpressionStmt& node) { resolve_expression(*node.expression); },
        [&](const VariableDeclStmt& node) {
            if (node.initializer) resolve_expression(*node.initializer);
            declare(node.name);
        },
        [&](const BlockStmt& node) {
            begin_scope();
            for (const auto& child : node.statements) resolve_statement(*child);
            end_scope();
        },
        [&](const IfStmt& node) {
            resolve_expression(*node.condition);
            resolve_statement(*node.then_branch);
            if (node.else_branch) resolve_statement(*node.else_branch);
        },
        [&](const WhileStmt& node) {
            resolve_expression(*node.condition);
            resolve_statement(*node.body);
        },
        [&](const ForEachStmt& node) {
            resolve_expression(*node.iterable);
            begin_scope();
            for (const auto& name : node.names) declare(name);
            resolve_statement(*node.body);
            end_scope();
        },
        [&](const FunctionDeclStmt& node) {
            declare(node.name);
            resolve_function(node);
        },
        [&](const ReflexStmt& node) {
            resolve_expression(*node.value);
            require_name(node.target, statement.span);
        },
        [&](const ReturnStmt& node) {
            if (node.value) resolve_expression(*node.value);
        },
        [&](const MakeConstantStmt& node) { require_name(node.name, statement.span); },
        [&](const ClassDeclStmt& node) {
            declare(node.name);
            if (node.superclass) require_name(*node.superclass, statement.span);
            for (const auto& method : node.methods) resolve_function(method, true);
        },
        [&](const ImportStmt&) {},
        // User-module exports are parsed but not linked yet. Validate these names
        // when module resolution is implemented, where imported/exported scopes exist.
        [&](const ExportStmt&) {},
        [&](const ThrowStmt&) {},
        [&](const TryStmt& node) {
            resolve_statement(*node.body);
            if (node.catch_body) {
                begin_scope();
                if (node.error_name) declare(*node.error_name);
                resolve_statement(*node.catch_body);
                end_scope();
            }
            if (node.finally_body) resolve_statement(*node.finally_body);
        },
        [&](const BreakStmt&) {},
        [&](const ContinueStmt&) {}
    }, statement.node);
}

void Resolver::resolve_expression(const Expr& expression) {
    std::visit(Overloaded{
        [&](const LiteralExpr&) {},
        [&](const VariableExpr& node) { require_name(node.name, expression.span); },
        [&](const AssignExpr& node) {
            resolve_expression(*node.value);
            require_name(node.name, expression.span);
        },
        [&](const UnaryExpr& node) { resolve_expression(*node.operand); },
        [&](const BinaryExpr& node) {
            resolve_expression(*node.left);
            resolve_expression(*node.right);
        },
        [&](const TernaryExpr& node) {
            resolve_expression(*node.condition);
            resolve_expression(*node.when_true);
            resolve_expression(*node.when_false);
        },
        [&](const CallExpr& node) {
            resolve_expression(*node.callee);
            for (const auto& argument : node.arguments) resolve_expression(*argument);
        },
        [&](const GetPropertyExpr& node) { resolve_expression(*node.object); },
        [&](const SetPropertyExpr& node) {
            resolve_expression(*node.object);
            resolve_expression(*node.value);
        },
        [&](const CastExpr& node) { resolve_expression(*node.value); },
        [&](const LambdaExpr& node) {
            begin_scope();
            for (const auto& parameter : node.parameters) declare(parameter);
            for (const auto& statement : node.body) resolve_statement(*statement);
            end_scope();
        },
        [&](const IndexExpr& node) {
            resolve_expression(*node.object);
            resolve_expression(*node.index);
        },
        [&](const AssignIndexExpr& node) {
            resolve_expression(*node.object);
            resolve_expression(*node.index);
            resolve_expression(*node.value);
        }
    }, expression.node);
}

}  // namespace novi
