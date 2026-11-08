#include "parser.hpp"

#include <algorithm>
#include <cctype>

namespace novi {

const Token& Parser::peek() const { return tokens_[current_]; }
const Token& Parser::previous() const { return tokens_[current_ - 1]; }
bool Parser::check(TokenKind kind) const { return peek().kind == kind; }
bool Parser::at_end() const { return check(TokenKind::End); }

bool Parser::match(TokenKind kind) {
    if (!check(kind)) return false;
    ++current_;
    return true;
}

bool Parser::check_name(const char* name) const {
    if (!check(TokenKind::Identifier)) return false;
    if (peek().lexeme.size() != std::char_traits<char>::length(name)) return false;
    for (std::size_t i = 0; i < peek().lexeme.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(peek().lexeme[i])) !=
            std::tolower(static_cast<unsigned char>(name[i]))) return false;
    }
    return true;
}

bool Parser::match_name(const char* name) {
    if (!check_name(name)) return false;
    ++current_;
    return true;
}

const Token& Parser::consume(TokenKind kind, const char* message) {
    if (check(kind)) return tokens_[current_++];
    report(peek(), message);
    throw ParseFailure{};
}

const Token& Parser::consume_name(const char* message) {
    return consume(TokenKind::Identifier, message);
}

void Parser::optional_semicolon() {
    if (match(TokenKind::Semicolon)) return;
    if (repl_mode_ && at_end()) return;
    report(peek(), "Expected ';' after statement");
    throw ParseFailure{};
}

void Parser::report(const Token& token, const std::string& message) {
    diagnostics_.push_back({DiagnosticCategory::Syntax, message, token.span});
}

ExprPtr Parser::make_expr(const Token& token, LiteralExpr value) {
    return std::make_unique<Expr>(token.span, std::move(value));
}

ExprPtr Parser::expression() { return assignment(); }

ExprPtr Parser::assignment() {
    ExprPtr left = conditional();
    if (match(TokenKind::PlusEqual) || match(TokenKind::MinusEqual)) {
        const Token op = previous();
        auto* variable = std::get_if<VariableExpr>(&left->node);
        if (auto* property = std::get_if<GetPropertyExpr>(&left->node)) {
            ExprPtr value = assignment();
            const SourceSpan span{left->span.start, value->span.end};
            return std::make_unique<Expr>(span, CompoundAssignPropertyExpr{
                std::move(property->object), property->name, std::move(value),
                op.kind == TokenKind::PlusEqual ? BinaryOperator::Add : BinaryOperator::Subtract});
        }
        if (auto* index = std::get_if<IndexExpr>(&left->node)) {
            ExprPtr value = assignment();
            const SourceSpan span{left->span.start, value->span.end};
            return std::make_unique<Expr>(span, CompoundAssignIndexExpr{
                std::move(index->object), std::move(index->index), std::move(value),
                op.kind == TokenKind::PlusEqual ? BinaryOperator::Add : BinaryOperator::Subtract});
        }
        if (!variable) {
            report(op, "Compound assignment requires a variable, property, or indexed target");
            throw ParseFailure{};
        }
        const std::string name = variable->name;
        ExprPtr value = assignment();
        const SourceSpan span{left->span.start, value->span.end};
        ExprPtr combined = std::make_unique<Expr>(span, BinaryExpr{
            op.kind == TokenKind::PlusEqual ? BinaryOperator::Add : BinaryOperator::Subtract,
            std::move(left), std::move(value)});
        return std::make_unique<Expr>(span, AssignExpr{name, std::move(combined)});
    }
    if (match(TokenKind::Equal)) {
        const Token equals = previous();
        ExprPtr value = assignment();
        if (auto* variable = std::get_if<VariableExpr>(&left->node)) {
            return std::make_unique<Expr>(SourceSpan{left->span.start, value->span.end},
                AssignExpr{variable->name, std::move(value)});
        }
        if (auto* property = std::get_if<GetPropertyExpr>(&left->node)) {
            const SourceSpan span{left->span.start, value->span.end};
            return std::make_unique<Expr>(span,
                SetPropertyExpr{std::move(property->object), property->name, std::move(value)});
        }
        if (auto* index = std::get_if<IndexExpr>(&left->node)) {
            return std::make_unique<Expr>(SourceSpan{left->span.start, value->span.end},
                AssignIndexExpr{std::move(index->object), std::move(index->index), std::move(value)});
        }
        report(equals, "Assignment target must be a variable");
        throw ParseFailure{};
    }
    return left;
}

ExprPtr Parser::conditional() {
    ExprPtr condition = logical_or();
    if (match(TokenKind::Question)) {
        ExprPtr yes = expression();
        consume(TokenKind::Colon, "Expected ':' in conditional expression");
        ExprPtr no = conditional();
        return std::make_unique<Expr>(SourceSpan{condition->span.start, no->span.end},
            TernaryExpr{std::move(condition), std::move(yes), std::move(no)});
    }
    return condition;
}

ExprPtr Parser::logical_or() {
    ExprPtr value = logical_and();
    while (match(TokenKind::Pipe) || match_name("or")) {
        ExprPtr right = logical_and();
        value = std::make_unique<Expr>(SourceSpan{value->span.start, right->span.end},
            BinaryExpr{BinaryOperator::LogicalOr, std::move(value), std::move(right)});
    }
    return value;
}

ExprPtr Parser::logical_and() {
    ExprPtr value = equality();
    while (match(TokenKind::Ampersand) || match_name("and")) {
        ExprPtr right = equality();
        value = std::make_unique<Expr>(SourceSpan{value->span.start, right->span.end},
            BinaryExpr{BinaryOperator::LogicalAnd, std::move(value), std::move(right)});
    }
    return value;
}

ExprPtr Parser::equality() {
    ExprPtr value = comparison();
    while (match(TokenKind::EqualEqual) || match(TokenKind::BangEqual)) {
        const Token op = previous();
        ExprPtr right = comparison();
        value = std::make_unique<Expr>(SourceSpan{value->span.start, right->span.end},
            BinaryExpr{op.kind == TokenKind::EqualEqual ? BinaryOperator::Equal : BinaryOperator::NotEqual,
                       std::move(value), std::move(right)});
    }
    return value;
}

ExprPtr Parser::comparison() {
    ExprPtr value = term();
    while (match(TokenKind::Less) || match(TokenKind::LessEqual) ||
           match(TokenKind::Greater) || match(TokenKind::GreaterEqual)) {
        const Token op = previous();
        ExprPtr right = term();
        BinaryOperator kind = BinaryOperator::Less;
        if (op.kind == TokenKind::LessEqual) kind = BinaryOperator::LessEqual;
        else if (op.kind == TokenKind::Greater) kind = BinaryOperator::Greater;
        else if (op.kind == TokenKind::GreaterEqual) kind = BinaryOperator::GreaterEqual;
        value = std::make_unique<Expr>(SourceSpan{value->span.start, right->span.end},
            BinaryExpr{kind, std::move(value), std::move(right)});
    }
    return value;
}

ExprPtr Parser::term() {
    ExprPtr value = factor();
    while (match(TokenKind::Plus) || match(TokenKind::Minus) || match(TokenKind::LogicalOr) ||
           match(TokenKind::Caret)) {
        const Token op = previous();
        ExprPtr right = factor();
        const auto kind = op.kind == TokenKind::Plus ? BinaryOperator::Add :
            op.kind == TokenKind::Minus ? BinaryOperator::Subtract :
            op.kind == TokenKind::LogicalOr ? BinaryOperator::BitwiseOr : BinaryOperator::BitwiseXor;
        value = std::make_unique<Expr>(SourceSpan{value->span.start, right->span.end},
            BinaryExpr{kind, std::move(value), std::move(right)});
    }
    return value;
}

ExprPtr Parser::factor() {
    ExprPtr value = unary();
    while (match(TokenKind::Star) || match(TokenKind::Slash) || match(TokenKind::FloorSlash) ||
           match(TokenKind::Percent) || match(TokenKind::LogicalAnd) ||
           match(TokenKind::ShiftLeft) || match(TokenKind::ShiftRight)) {
        const Token op = previous();
        ExprPtr right = unary();
        BinaryOperator kind = BinaryOperator::Multiply;
        if (op.kind == TokenKind::Slash) kind = BinaryOperator::Divide;
        else if (op.kind == TokenKind::FloorSlash) kind = BinaryOperator::FloorDivide;
        else if (op.kind == TokenKind::Percent) kind = BinaryOperator::Modulo;
        else if (op.kind == TokenKind::LogicalAnd) kind = BinaryOperator::BitwiseAnd;
        else if (op.kind == TokenKind::ShiftLeft) kind = BinaryOperator::ShiftLeft;
        else if (op.kind == TokenKind::ShiftRight) kind = BinaryOperator::ShiftRight;
        value = std::make_unique<Expr>(SourceSpan{value->span.start, right->span.end},
            BinaryExpr{kind, std::move(value), std::move(right)});
    }
    return value;
}

ExprPtr Parser::unary() {
    if (match(TokenKind::Minus) || match(TokenKind::Bang) || match_name("not") || match(TokenKind::Tilde)) {
        const Token op = previous();
        ExprPtr operand = unary();
        const auto kind = op.kind == TokenKind::Minus ? UnaryOperator::Negate :
            op.kind == TokenKind::Tilde ? UnaryOperator::BitNot : UnaryOperator::Not;
        return std::make_unique<Expr>(SourceSpan{op.span.start, operand->span.end},
            UnaryExpr{kind, std::move(operand)});
    }
    return call();
}

ExprPtr Parser::call() {
    ExprPtr value = primary();
    for (;;) {
        if (match(TokenKind::LeftParen)) {
            std::vector<ExprPtr> arguments;
            if (!check(TokenKind::RightParen)) {
                do { arguments.push_back(expression()); } while (match(TokenKind::Comma));
            }
            const Token& close = consume(TokenKind::RightParen, "Expected ')' after arguments");
            if (auto* property = std::get_if<GetPropertyExpr>(&value->node)) {
                if (property->name == "print") {
                    if (auto* object = std::get_if<VariableExpr>(&property->object->node);
                        object && object->name == "console") {
                        value = std::make_unique<Expr>(value->span, VariableExpr{"print"});
                    }
                }
            }
            value = std::make_unique<Expr>(SourceSpan{value->span.start, close.span.end},
                CallExpr{std::move(value), std::move(arguments)});
        } else if (match(TokenKind::Dot)) {
            const Token& name = consume_name("Expected property name after '.'");
            value = std::make_unique<Expr>(SourceSpan{value->span.start, name.span.end},
                GetPropertyExpr{std::move(value), name.lexeme});
        } else if (match(TokenKind::LeftBracket)) {
            ExprPtr index = expression();
            const Token& close = consume(TokenKind::RightBracket, "Expected ']' after index");
            value = std::make_unique<Expr>(SourceSpan{value->span.start, close.span.end},
                IndexExpr{std::move(value), std::move(index)});
        } else if (match(TokenKind::Colon)) {
            const Token& type = consume_name("Expected type name after ':'");
            value = std::make_unique<Expr>(SourceSpan{value->span.start, type.span.end},
                CastExpr{std::move(value), type.lexeme});
        } else break;
    }
    return value;
}

ExprPtr Parser::primary() {
    if (match_name("function")) {
        const Token start = previous();
        consume(TokenKind::LeftBrace, "Expected '{' before lambda parameters");
        std::vector<std::string> parameters;
        if (!check(TokenKind::RightBrace)) {
            do {
                const Token& parameter = consume_name("Expected lambda parameter");
                if (std::find(parameters.begin(), parameters.end(), parameter.lexeme) != parameters.end()) {
                    report(parameter, "Duplicate parameter '" + parameter.lexeme + "'");
                    throw ParseFailure{};
                }
                parameters.push_back(parameter.lexeme);
            }
            while (match(TokenKind::Comma));
        }
        consume(TokenKind::RightBrace, "Expected '}' after lambda parameters");
        if (!match_name("then")) {
            report(peek(), "Expected 'then' before lambda body");
            throw ParseFailure{};
        }
        std::vector<StmtPtr> body;
        const int enclosing_loop_depth = loop_depth_;
        loop_depth_ = 0;
        ++function_depth_;
        try {
            if (check(TokenKind::LeftBrace)) {
                consume(TokenKind::LeftBrace, "Expected lambda body");
                body = block_contents();
            } else {
                StmtPtr command = statement();
                body.push_back(std::move(command));
            }
        } catch (...) { --function_depth_; loop_depth_ = enclosing_loop_depth; throw; }
        --function_depth_;
        loop_depth_ = enclosing_loop_depth;
        return std::make_unique<Expr>(SourceSpan{start.span.start, previous().span.end},
            LambdaExpr{std::move(parameters), std::move(body)});
    }
    if (match(TokenKind::Number)) return make_expr(previous(), LiteralExpr{previous().number});
    if (match(TokenKind::String)) return make_expr(previous(), LiteralExpr{previous().lexeme});
    if (match(TokenKind::LeftBracket)) {
        report(previous(), "Collection literal syntax has not been decided; use dictionary() for now");
        throw ParseFailure{};
    }
    if (match_name("true")) return make_expr(previous(), LiteralExpr{true});
    if (match_name("false")) return make_expr(previous(), LiteralExpr{false});
    if (match_name("null") || match_name("none")) return make_expr(previous(), LiteralExpr{std::monostate{}});
    if (match(TokenKind::Identifier)) return std::make_unique<Expr>(previous().span, VariableExpr{previous().lexeme});
    if (match(TokenKind::LeftParen)) {
        const Token opening = previous();
        ExprPtr result = expression();
        const Token& closing = consume(TokenKind::RightParen, "Expected ')' after expression");
        result->span = {opening.span.start, closing.span.end};
        return result;
    }
    report(peek(), "Expected an expression");
    throw ParseFailure{};
}

std::vector<std::string> Parser::parameter_list() {
    std::vector<std::string> parameters;
    consume(TokenKind::LeftParen, "Expected '('");
    if (!check(TokenKind::RightParen)) {
        do {
            const Token& parameter = consume_name("Expected parameter name");
            if (std::find(parameters.begin(), parameters.end(), parameter.lexeme) != parameters.end()) {
                report(parameter, "Duplicate parameter '" + parameter.lexeme + "'");
                throw ParseFailure{};
            }
            parameters.push_back(parameter.lexeme);
        }
        while (match(TokenKind::Comma));
    }
    consume(TokenKind::RightParen, "Expected ')' after parameters");
    return parameters;
}

std::vector<StmtPtr> Parser::block_contents() {
    std::vector<StmtPtr> statements;
    while (!check(TokenKind::RightBrace) && !at_end()) statements.push_back(declaration());
    consume(TokenKind::RightBrace, "Expected '}' after block");
    return statements;
}

StmtPtr Parser::block_statement() {
    const Token opening = consume(TokenKind::LeftBrace, "Expected '{'");
    auto statements = block_contents();
    const auto end = previous().span.end;
    return std::make_unique<Stmt>(SourceSpan{opening.span.start, end}, BlockStmt{std::move(statements)});
}

StmtPtr Parser::variable_declaration(bool is_constant, bool, bool) {
    const Token& name = consume_name("Expected variable name");
    std::string fixed_type;
    ExprPtr initializer;
    const bool has_type_marker = match(TokenKind::Colon);
    if (has_type_marker) {
        if (match(TokenKind::Equal)) initializer = expression();
        else {
            fixed_type = consume_name("Expected type name or '=' after ':'").lexeme;
            if (match(TokenKind::Equal)) initializer = expression();
        }
    }
    if (!has_type_marker || (!initializer && fixed_type.empty())) {
        report(peek(), "A variable declaration needs a type annotation and an initial value");
        throw ParseFailure{};
    }
    // An inline lambda command already consumes the declaration terminator.
    // A block-bodied lambda still needs the declaration's own semicolon.
    const bool lambda_terminated = initializer &&
        std::holds_alternative<LambdaExpr>(initializer->node) &&
        previous().kind == TokenKind::Semicolon;
    if (!lambda_terminated) optional_semicolon();
    return std::make_unique<Stmt>(name.span,
        VariableDeclStmt{name.lexeme, std::move(initializer), std::move(fixed_type), is_constant});
}

StmtPtr Parser::function_declaration(bool is_async, bool is_static, bool is_private,
                                     std::vector<std::string> attributes) {
    const Token& name = consume_name("Expected function name");
    auto parameters = parameter_list();
    consume(TokenKind::LeftBrace, "Expected '{' before function body");
    const int enclosing_loop_depth = loop_depth_;
    loop_depth_ = 0;
    ++function_depth_;
    std::vector<StmtPtr> body;
    try { body = block_contents(); }
    catch (...) { --function_depth_; loop_depth_ = enclosing_loop_depth; throw; }
    --function_depth_;
    loop_depth_ = enclosing_loop_depth;
    FunctionDeclStmt function{name.lexeme, std::move(parameters), std::move(body),
                              is_async, is_static, is_private, std::move(attributes)};
    return std::make_unique<Stmt>(name.span, std::move(function));
}

StmtPtr Parser::class_declaration(std::vector<std::string>) {
    const Token& name = consume_name("Expected class name");
    ClassDeclStmt klass;
    klass.name = name.lexeme;
    consume(TokenKind::LeftBrace, "Expected '{' before class body");
    while (!check(TokenKind::RightBrace) && !at_end()) {
        bool is_async = false, is_static = false, is_private = false;
        if (match_name("async")) is_async = true;
        if (match_name("static")) is_static = true;
        if (match_name("private") || match(TokenKind::Hash)) is_private = true;
        else match_name("public");
        if (match_name("function")) {
            if (is_static) {
                report(previous(), "Static methods are not yet supported");
                throw ParseFailure{};
            }
            auto method = function_declaration(is_async, is_static, is_private);
            if (is_private && std::get<FunctionDeclStmt>(method->node).name == "init") {
                report(previous(), "Private constructors are not yet supported");
                throw ParseFailure{};
            }
            klass.methods.push_back(std::get<FunctionDeclStmt>(std::move(method->node)));
        } else if (match_name("vari")) {
            if (is_static) {
                report(previous(), "Static class fields are not yet supported");
                throw ParseFailure{};
            }
            auto field = variable_declaration(false, is_private, is_static);
            auto& decl = std::get<VariableDeclStmt>(field->node);
            if (std::any_of(klass.fields.begin(), klass.fields.end(),
                            [&](const FieldDeclStmt& item) { return item.name == decl.name; })) {
                report(previous(), "Duplicate class field '" + decl.name + "'");
                throw ParseFailure{};
            }
            klass.fields.push_back({decl.name, std::move(decl.initializer), decl.fixed_type,
                                    decl.is_constant, is_static, is_private});
        } else if (match_name("let")) {
            if (is_static) {
                report(previous(), "Static class fields are not yet supported");
                throw ParseFailure{};
            }
            auto field = variable_declaration(true, is_private, is_static);
            auto& decl = std::get<VariableDeclStmt>(field->node);
            if (std::any_of(klass.fields.begin(), klass.fields.end(),
                            [&](const FieldDeclStmt& item) { return item.name == decl.name; })) {
                report(previous(), "Duplicate class field '" + decl.name + "'");
                throw ParseFailure{};
            }
            if (!decl.initializer) {
                report(previous(), "A constant field needs an initial value");
                throw ParseFailure{};
            }
            klass.fields.push_back({decl.name, std::move(decl.initializer), decl.fixed_type,
                                    decl.is_constant, is_static, is_private});
        } else {
            report(peek(), "Expected a class field or function");
            throw ParseFailure{};
        }
    }
    consume(TokenKind::RightBrace, "Expected '}' after class body");
    return std::make_unique<Stmt>(name.span, std::move(klass));
}

StmtPtr Parser::if_statement() {
    const Token start = previous();
    consume(TokenKind::LeftParen, "Expected '(' after is/if");
    ExprPtr condition = expression();
    consume(TokenKind::RightParen, "Expected ')' after condition");
    match_name("then");
    StmtPtr yes = check(TokenKind::LeftBrace) ? block_statement() : statement();
    StmtPtr no;
    if (match_name("also") || match_name("elif")) no = if_statement();
    else if (match_name("isnothing") || match_name("else")) {
        match_name("then");
        no = check(TokenKind::LeftBrace) ? block_statement() : statement();
    }
    const SourceSpan span{start.span.start, (no ? no->span.end : yes->span.end)};
    if (contains_reflex(*yes) && !no)
        return std::make_unique<Stmt>(span, WhileStmt{std::move(condition), std::move(yes)});
    return std::make_unique<Stmt>(span, IfStmt{std::move(condition), std::move(yes), std::move(no)});
}

StmtPtr Parser::while_statement() {
    const Token start = previous();
    consume(TokenKind::LeftParen, "Expected '(' after while");
    ExprPtr condition = expression();
    consume(TokenKind::RightParen, "Expected ')' after condition");
    ++loop_depth_;
    StmtPtr body;
    try { body = check(TokenKind::LeftBrace) ? block_statement() : statement(); }
    catch (...) { --loop_depth_; throw; }
    --loop_depth_;
    return std::make_unique<Stmt>(SourceSpan{start.span.start, body->span.end},
        WhileStmt{std::move(condition), std::move(body)});
}

StmtPtr Parser::foreach_statement() {
    const Token start = previous();
    consume(TokenKind::LeftParen, "Expected '(' after foreach");
    std::vector<std::string> names;
    names.push_back(consume_name("Expected loop variable").lexeme);
    if (match(TokenKind::Comma)) {
        names.push_back(consume_name("Expected second loop variable").lexeme);
        if (names[0] == names[1]) {
            report(previous(), "foreach key and value variables must have different names");
            throw ParseFailure{};
        }
    }
    if (!match_name("in")) {
        report(peek(), "Expected 'in' after foreach variables");
        throw ParseFailure{};
    }
    ExprPtr iterable = expression();
    consume(TokenKind::RightParen, "Expected ')' after foreach iterable");
    ++loop_depth_;
    StmtPtr body;
    try { body = check(TokenKind::LeftBrace) ? block_statement() : statement(); }
    catch (...) { --loop_depth_; throw; }
    --loop_depth_;
    return std::make_unique<Stmt>(SourceSpan{start.span.start, body->span.end},
        ForEachStmt{std::move(names), std::move(iterable), std::move(body)});
}

StmtPtr Parser::import_statement() {
    consume(TokenKind::LeftBrace, "Expected '{' in import");
    std::vector<std::string> imported;
    if (!check(TokenKind::RightBrace)) do { imported.push_back(consume_name("Expected imported name").lexeme); }
    while (match(TokenKind::Comma));
    consume(TokenKind::RightBrace, "Expected '}' in import");
    if (!match_name("from")) {
        report(peek(), "Expected 'from' in import");
        throw ParseFailure{};
    }
    consume(TokenKind::LeftBrace, "Expected '{' before module name");
    const Token& module = consume_name("Expected module name");
    consume(TokenKind::RightBrace, "Expected '}' after module name");
    if (module.lexeme != "novi") {
        report(module, "Only the built-in {novi} module is available at this stage");
        throw ParseFailure{};
    }
    consume(TokenKind::Semicolon, "Import/export declarations must end with ';'");
    std::vector<std::pair<std::string, std::string>> names;
    for (const auto& name : imported) names.emplace_back(name, name);
    return std::make_unique<Stmt>(module.span, ImportStmt{module.lexeme, std::move(names)});
}

StmtPtr Parser::export_statement() {
    consume(TokenKind::LeftBrace, "Expected '{' in export");
    std::vector<std::string> names;
    if (!check(TokenKind::RightBrace)) do { names.push_back(consume_name("Expected exported name").lexeme); }
    while (match(TokenKind::Comma));
    consume(TokenKind::RightBrace, "Expected '}' in export");
    std::string alias;
    if (match_name("like")) {
        consume(TokenKind::LeftParen, "Expected '(' before export alias");
        alias = consume(TokenKind::String, "Expected string alias").lexeme;
        consume(TokenKind::RightParen, "Expected ')' after export alias");
    }
    consume(TokenKind::Semicolon, "Import/export declarations must end with ';'");
    return std::make_unique<Stmt>(previous().span, ExportStmt{std::move(names), std::move(alias)});
}

StmtPtr Parser::declaration(std::vector<std::string> attributes) {
    try {
        if (match(TokenKind::At)) {
            attributes.push_back(consume_name("Expected annotation name after '@'").lexeme);
            return declaration(std::move(attributes));
        }
        bool is_async = match_name("async");
        if (match_name("vari")) return variable_declaration(false);
        if (match_name("let")) return variable_declaration(true);
        if (match_name("constant")) {
            const Token& name = consume_name("Expected an existing variable name after 'constant'");
            optional_semicolon();
            return std::make_unique<Stmt>(name.span, MakeConstantStmt{name.lexeme});
        }
        if (match_name("function")) return function_declaration(is_async, false, false, std::move(attributes));
        if (match_name("class")) return class_declaration(std::move(attributes));
        if (match_name("import")) return import_statement();
        if (match_name("export")) return export_statement();
        if (is_async) { report(previous(), "'async' must precede a function declaration"); throw ParseFailure{}; }
        return statement();
    } catch (const ParseFailure&) {
        while (!at_end() && !check(TokenKind::Semicolon) && !check(TokenKind::RightBrace)) ++current_;
        match(TokenKind::Semicolon);
        throw;
    }
}

StmtPtr Parser::statement(std::vector<std::string>) {
    if (match_name("is") || match_name("if")) return if_statement();
    if (match_name("while")) return while_statement();
    if (match_name("during")) return while_statement();
    if (match_name("foreach")) return foreach_statement();
    if (match_name("try")) {
        const Token start = previous();
        StmtPtr body = block_statement();
        std::optional<std::string> error_name;
        StmtPtr catch_body;
        StmtPtr finally_body;
        if (match_name("catch")) {
            consume(TokenKind::LeftParen, "Expected '(' after catch");
            error_name = consume_name("Expected catch variable name").lexeme;
            consume(TokenKind::RightParen, "Expected ')' after catch variable");
            catch_body = block_statement();
        }
        if (match_name("finally")) finally_body = block_statement();
        if (!catch_body && !finally_body) {
            report(peek(), "A try block requires catch(error), finally, or both");
            throw ParseFailure{};
        }
        const SourceLocation end = finally_body ? finally_body->span.end : catch_body->span.end;
        return std::make_unique<Stmt>(SourceSpan{start.span.start, end},
            TryStmt{std::move(body), std::move(error_name), std::move(catch_body), std::move(finally_body)});
    }
    if (match_name("throw")) {
        const Token start = previous();
        const std::string error_kind = consume_name("Expected an error kind after throw").lexeme;
        optional_semicolon();
        return std::make_unique<Stmt>(SourceSpan{start.span.start, previous().span.end}, ThrowStmt{error_kind});
    }
    if (match_name("break")) {
        const Token start = previous();
        if (loop_depth_ == 0) {
            report(start, "break is only valid inside a loop");
            throw ParseFailure{};
        }
        optional_semicolon();
        return std::make_unique<Stmt>(SourceSpan{start.span.start, previous().span.end}, BreakStmt{});
    }
    if (match_name("continue")) {
        const Token start = previous();
        if (loop_depth_ == 0) {
            report(start, "continue is only valid inside a loop");
            throw ParseFailure{};
        }
        optional_semicolon();
        return std::make_unique<Stmt>(SourceSpan{start.span.start, previous().span.end}, ContinueStmt{});
    }
    if (match_name("output") || match_name("return")) {
        const Token start = previous();
        if (function_depth_ == 0) {
            report(start, "return is only valid inside a function");
            throw ParseFailure{};
        }
        ExprPtr value;
        if (!check(TokenKind::Semicolon) && !check(TokenKind::RightBrace) && !at_end()) value = expression();
        optional_semicolon();
        return std::make_unique<Stmt>(start.span, ReturnStmt{std::move(value)});
    }
    if (match_name("reflex")) {
        consume(TokenKind::LeftParen, "Expected '(' after reflex");
        ExprPtr value = expression();
        consume(TokenKind::RightParen, "Expected ')' after reflex value");
        const Token& target = consume_name("Expected target variable after reflex(value)");
        optional_semicolon();
        return std::make_unique<Stmt>(target.span, ReflexStmt{std::move(value), target.lexeme});
    }
    if (check(TokenKind::LeftBrace)) return block_statement();
    ExprPtr value = expression();
    const auto span = value->span;
    optional_semicolon();
    return std::make_unique<Stmt>(span, ExpressionStmt{std::move(value)});
}

bool Parser::contains_reflex(const Stmt& value) const {
    if (std::holds_alternative<ReflexStmt>(value.node)) return true;
    if (const auto* block = std::get_if<BlockStmt>(&value.node))
        return std::any_of(block->statements.begin(), block->statements.end(),
            [this](const StmtPtr& child) { return contains_reflex(*child); });
    if (const auto* branch = std::get_if<IfStmt>(&value.node))
        return contains_reflex(*branch->then_branch) ||
            (branch->else_branch && contains_reflex(*branch->else_branch));
    return false;
}

ParseResult Parser::parse_program() {
    auto program = std::make_unique<Program>();
    program->echo_last_expression = false;
    while (!at_end()) {
        const std::size_t before = current_;
        try { program->statements.push_back(declaration()); }
        catch (const ParseFailure&) {}
        if (current_ == before) ++current_;
    }
    return {std::move(program), std::move(diagnostics_)};
}

ParseResult Parser::parse_line() {
    repl_mode_ = true;
    ParseResult result = parse_program();
    if (result.program) result.program->echo_last_expression = true;
    return result;
}

}  // namespace novi
