#pragma once

#include <string>
#include <unordered_set>
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
    void declare(const std::string& name);
    void require_name(const std::string& name, const SourceSpan& span);
    void begin_scope();
    void end_scope();

    std::vector<std::unordered_set<std::string>> scopes_;
    Diagnostics diagnostics_;
};

}  // namespace novi
