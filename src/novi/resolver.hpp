#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "ast.hpp"
#include "diagnostic.hpp"

namespace novi {

class Resolver {
public:
    Diagnostics resolve(const Program& program);

private:
    void resolve_statement(const Stmt& statement);
    void resolve_expression(const Expr& expression);
    void resolve_function(const FunctionDeclStmt& function, bool is_method = false);
    struct Binding {
        std::string fixed_type;
        bool is_constant = false;
    };
    void declare(const std::string& name, std::string fixed_type = {}, bool is_constant = false);
    void require_name(const std::string& name, const SourceSpan& span);
    Binding* find_binding(const std::string& name);
    const Binding* find_binding(const std::string& name) const;
    std::string known_type(const Expr& expression) const;
    bool is_compatible(const std::string& expected, const std::string& actual) const;
    void begin_scope();
    void end_scope();

    std::vector<std::unordered_map<std::string, Binding>> scopes_;
    Diagnostics diagnostics_;
};

}  // namespace novi
