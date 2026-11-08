#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ast.hpp"
#include "diagnostic.hpp"
#include "lexer.hpp"

namespace novi {

struct ParseResult {
    std::unique_ptr<Program> program;
    Diagnostics diagnostics;
};

class Parser {
public:
    Parser(const SourceFile& source, std::vector<Token> tokens)
        : source_(source), tokens_(std::move(tokens)) {}
    ParseResult parse_program();
    ParseResult parse_line();

private:
    struct ParseFailure {};
    const Token& peek() const;
    const Token& previous() const;
    bool match(TokenKind kind);
    bool check(TokenKind kind) const;
    bool check_name(const char* name) const;
    bool match_name(const char* name);
    const Token& consume(TokenKind kind, const char* message);
    const Token& consume_name(const char* message);
    bool at_end() const;
    void optional_semicolon();
    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr conditional();
    ExprPtr logical_or();
    ExprPtr logical_and();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr call();
    ExprPtr postfix();
    ExprPtr primary();
    StmtPtr declaration(std::vector<std::string> attributes = {});
    StmtPtr statement(std::vector<std::string> attributes = {});
    StmtPtr variable_declaration(bool is_constant, bool is_private = false, bool is_static = false);
    StmtPtr function_declaration(bool is_async = false, bool is_static = false, bool is_private = false,
                                 std::vector<std::string> attributes = {});
    StmtPtr class_declaration(std::vector<std::string> attributes = {});
    StmtPtr if_statement();
    StmtPtr while_statement();
    StmtPtr foreach_statement();
    StmtPtr block_statement();
    StmtPtr import_statement();
    StmtPtr export_statement();
    std::vector<StmtPtr> block_contents();
    std::vector<std::string> parameter_list();
    bool contains_reflex(const Stmt& statement) const;
    ExprPtr make_expr(const Token& token, LiteralExpr value);
    void report(const Token& token, const std::string& message);

    const SourceFile& source_;
    std::vector<Token> tokens_;
    std::size_t current_ = 0;
    bool repl_mode_ = false;
    int loop_depth_ = 0;
    int function_depth_ = 0;
    Diagnostics diagnostics_;
};

}  // namespace novi
