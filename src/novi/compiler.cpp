#include "compiler.hpp"
#include "resolver.hpp"

#include <type_traits>
#include <utility>

namespace novi {
namespace {

template <class... Ts> struct Overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;

class FunctionCompiler {
public:
    explicit FunctionCompiler(std::shared_ptr<FunctionPrototype> function)
        : function_(std::move(function)) {}

    std::shared_ptr<FunctionPrototype> compile(const std::vector<StmtPtr>& statements,
                                                const SourceSpan& span) {
        for (const auto& statement : statements) compile_statement(*statement);
        emit(OpCode::Null, 0, span);
        emit(OpCode::Return, 0, span);
        return function_;
    }

    std::shared_ptr<FunctionPrototype> compile_initializer(const std::vector<FieldDeclStmt>& fields,
                                                            const FunctionDeclStmt* initializer,
                                                            const SourceSpan& span) {
        for (const auto& field : fields) {
            emit(OpCode::GetName, add_constant(std::string("this")), span);
            if (field.initializer) compile_expression(*field.initializer);
            else emit(OpCode::Null, 0, span);
            emit(OpCode::SetProperty, add_constant(field.name), span);
            emit(OpCode::Pop, 0, span);
        }
        if (initializer)
            for (const auto& statement : initializer->body) compile_statement(*statement);
        emit(OpCode::Null, 0, span);
        emit(OpCode::Return, 0, span);
        return function_;
    }

    std::shared_ptr<FunctionPrototype> compile_program(const Program& program) {
        const bool echo = program.echo_last_expression && !program.statements.empty() &&
            std::holds_alternative<ExpressionStmt>(program.statements.back()->node);
        const std::size_t end = echo ? program.statements.size() - 1 : program.statements.size();
        for (std::size_t i = 0; i < end; ++i) compile_statement(*program.statements[i]);
        if (echo) {
            const auto& last = std::get<ExpressionStmt>(program.statements.back()->node);
            compile_expression(*last.expression);
            emit(OpCode::Return, 0, program.statements.back()->span);
        } else {
            emit(OpCode::Null, 0, {});
            emit(OpCode::Return, 0, {});
        }
        return function_;
    }

private:
    struct LoopContext {
        int continue_target = 0;
        int scope_depth = 0;
        int exception_depth = 0;
        std::size_t finalizer_depth = 0;
        std::vector<int> break_jumps;
    };

    void exit_scopes_to(int target_depth, const SourceSpan& span) {
        for (int depth = scope_depth_; depth > target_depth; --depth)
            emit(OpCode::ExitScope, 0, span);
    }

    void emit_exception_pops_to(int target_depth, const SourceSpan& span) {
        for (int depth = exception_depth_; depth > target_depth; --depth)
            emit(OpCode::PopExceptionHandler, 0, span);
    }

    void compile_finalizers_to(std::size_t target_depth, int target_exception_depth,
                               const SourceSpan& span) {
        const auto finalizers = active_finalizers_;
        emit_exception_pops_to(target_exception_depth, span);
        const int saved_exception_depth = exception_depth_;
        exception_depth_ = target_exception_depth;
        for (std::size_t offset = finalizers.size(); offset > target_depth; --offset) {
            active_finalizers_.resize(offset - 1);
            compile_statement(*finalizers[offset - 1]);
        }
        active_finalizers_ = finalizers;
        exception_depth_ = saved_exception_depth;
    }

    void emit(OpCode op, int operand, const SourceSpan& span, int operand2 = 0,
              int operand3 = 0, int operand4 = 0) {
        function_->chunk.code.push_back({op, operand, span, operand2, operand3, operand4});
    }

    int add_constant(Constant value) {
        function_->chunk.constants.push_back(std::move(value));
        return static_cast<int>(function_->chunk.constants.size() - 1);
    }

    int add_function(std::shared_ptr<FunctionPrototype> value) {
        function_->chunk.functions.push_back(std::move(value));
        return static_cast<int>(function_->chunk.functions.size() - 1);
    }

    void compile_function(const FunctionDeclStmt& declaration, const SourceSpan& span,
                          std::shared_ptr<FunctionPrototype>& output) {
        auto nested = std::make_shared<FunctionPrototype>();
        nested->name = declaration.name;
        nested->parameters = declaration.parameters;
        nested->declaration_span = span;
        FunctionCompiler nested_compiler(nested);
        output = nested_compiler.compile(declaration.body, span);
    }

    void compile_statement(const Stmt& statement) {
        std::visit(Overloaded{
            [&](const ExpressionStmt& node) {
                compile_expression(*node.expression);
                emit(OpCode::Pop, 0, statement.span);
            },
            [&](const VariableDeclStmt& node) {
                if (node.initializer) compile_expression(*node.initializer);
                else emit(OpCode::Null, 0, statement.span);
                if (!node.fixed_type.empty()) {
                    emit(node.is_constant ? OpCode::DefineTypedConstantName : OpCode::DefineTypedName,
                         add_constant(node.name), statement.span, add_constant(node.fixed_type));
                } else {
                    emit(node.is_constant ? OpCode::DefineConstantName : OpCode::DefineName,
                         add_constant(node.name), statement.span);
                }
            },
            [&](const MakeConstantStmt& node) {
                emit(OpCode::FreezeName, add_constant(node.name), statement.span);
            },
            [&](const ReflexStmt& node) {
                compile_expression(*node.value);
                emit(OpCode::SetName, add_constant(node.target), statement.span);
                emit(OpCode::Pop, 0, statement.span);
            },
            [&](const BlockStmt& node) {
                emit(OpCode::EnterScope, 0, statement.span);
                ++scope_depth_;
                for (const auto& child : node.statements) compile_statement(*child);
                --scope_depth_;
                emit(OpCode::ExitScope, 0, statement.span);
            },
            [&](const IfStmt& node) {
                compile_expression(*node.condition);
                const int false_jump = static_cast<int>(function_->chunk.code.size());
                emit(OpCode::JumpIfFalse, -1, node.condition->span);
                emit(OpCode::Pop, 0, node.condition->span);
                compile_statement(*node.then_branch);
                const int end_jump = static_cast<int>(function_->chunk.code.size());
                emit(OpCode::Jump, -1, statement.span);
                patch_jump(false_jump);
                emit(OpCode::Pop, 0, node.condition->span);
                if (node.else_branch) compile_statement(*node.else_branch);
                patch_jump(end_jump);
            },
            [&](const WhileStmt& node) {
                const int loop_start = static_cast<int>(function_->chunk.code.size());
                compile_expression(*node.condition);
                const int exit_jump = static_cast<int>(function_->chunk.code.size());
                emit(OpCode::JumpIfFalse, -1, node.condition->span);
                emit(OpCode::Pop, 0, node.condition->span);
                loops_.push_back({loop_start, scope_depth_, exception_depth_, active_finalizers_.size(), {}});
                compile_statement(*node.body);
                LoopContext loop = std::move(loops_.back());
                loops_.pop_back();
                emit(OpCode::Jump, loop_start, statement.span);
                patch_jump(exit_jump);
                emit(OpCode::Pop, 0, node.condition->span);
                for (const int jump : loop.break_jumps) patch_jump(jump);
            },
            [&](const ForEachStmt& node) {
                emit(OpCode::EnterScope, 0, statement.span);
                ++scope_depth_;
                compile_expression(*node.iterable);
                emit(OpCode::IteratorInit, 0, node.iterable->span);
                const int loop_start = static_cast<int>(function_->chunk.code.size());
                const int next_instruction = static_cast<int>(function_->chunk.code.size());
                emit(OpCode::IteratorNext, add_constant(node.names.front()), statement.span,
                     -1, node.names.size() > 1 ? add_constant(node.names[1]) : -1);
                loops_.push_back({loop_start, scope_depth_, exception_depth_, active_finalizers_.size(), {}});
                compile_statement(*node.body);
                LoopContext loop = std::move(loops_.back());
                loops_.pop_back();
                emit(OpCode::Jump, loop_start, statement.span);
                function_->chunk.code[static_cast<std::size_t>(next_instruction)].operand2 =
                    static_cast<int>(function_->chunk.code.size());
                const int cleanup_target = static_cast<int>(function_->chunk.code.size());
                emit(OpCode::Pop, 0, statement.span);
                emit(OpCode::ExitScope, 0, statement.span);
                --scope_depth_;
                for (const int jump : loop.break_jumps)
                    function_->chunk.code[static_cast<std::size_t>(jump)].operand = cleanup_target;
            },
            [&](const FunctionDeclStmt& node) {
                std::shared_ptr<FunctionPrototype> nested;
                compile_function(node, statement.span, nested);
                emit(OpCode::MakeClosure, add_function(std::move(nested)), statement.span);
                emit(OpCode::DefineName, add_constant(node.name), statement.span);
            },
            [&](const ReturnStmt& node) {
                if (node.value) compile_expression(*node.value);
                else emit(OpCode::Null, 0, statement.span);
                compile_finalizers_to(0, 0, statement.span);
                emit(OpCode::Return, 0, statement.span);
            },
            [&](const BreakStmt&) {
                compile_finalizers_to(loops_.back().finalizer_depth,
                                      loops_.back().exception_depth, statement.span);
                exit_scopes_to(loops_.back().scope_depth, statement.span);
                const int jump = static_cast<int>(function_->chunk.code.size());
                emit(OpCode::Jump, -1, statement.span);
                loops_.back().break_jumps.push_back(jump);
            },
            [&](const ContinueStmt&) {
                compile_finalizers_to(loops_.back().finalizer_depth,
                                      loops_.back().exception_depth, statement.span);
                exit_scopes_to(loops_.back().scope_depth, statement.span);
                emit(OpCode::Jump, loops_.back().continue_target, statement.span);
            },
            [&](const ThrowStmt& node) {
                emit(OpCode::Throw, add_constant(node.error_kind), statement.span);
            },
            [&](const TryStmt& node) {
                const int exception_id = next_exception_id_++;
                const int handler = static_cast<int>(function_->chunk.code.size());
                emit(OpCode::PushExceptionHandler, -1, statement.span,
                     node.error_name ? add_constant(*node.error_name) : -1, -1, exception_id);
                ++exception_depth_;
                if (node.finally_body) active_finalizers_.push_back(node.finally_body.get());
                compile_statement(*node.body);
                emit(OpCode::PopExceptionHandler, 0, statement.span);
                --exception_depth_;
                if (node.finally_body) {
                    active_finalizers_.pop_back();
                    compile_statement(*node.finally_body);
                }
                const int end_jump = static_cast<int>(function_->chunk.code.size());
                emit(OpCode::Jump, -1, statement.span);
                std::vector<int> end_jumps{end_jump};
                if (node.catch_body) {
                    function_->chunk.code[static_cast<std::size_t>(handler)].operand =
                        static_cast<int>(function_->chunk.code.size());
                    ++exception_depth_;
                    // The VM creates a scope for the catch binding on entry.
                    ++scope_depth_;
                    if (node.finally_body) active_finalizers_.push_back(node.finally_body.get());
                    compile_statement(*node.catch_body);
                    emit(OpCode::ExitScope, 0, statement.span);
                    --scope_depth_;
                    emit(OpCode::PopExceptionHandler, 0, statement.span);
                    --exception_depth_;
                    if (node.finally_body) {
                        active_finalizers_.pop_back();
                        compile_statement(*node.finally_body);
                    }
                    const int catch_end = static_cast<int>(function_->chunk.code.size());
                    emit(OpCode::Jump, -1, statement.span);
                    end_jumps.push_back(catch_end);
                }
                if (node.finally_body) {
                    function_->chunk.code[static_cast<std::size_t>(handler)].operand3 =
                        static_cast<int>(function_->chunk.code.size());
                    compile_statement(*node.finally_body);
                    emit(OpCode::Rethrow, exception_id, statement.span);
                }
                for (const int jump : end_jumps) patch_jump(jump);
            },
            [&](const ClassDeclStmt& node) {
                if (node.superclass) emit(OpCode::GetName, add_constant(*node.superclass), statement.span);
                else emit(OpCode::Null, 0, statement.span);
                ClassPrototype klass;
                klass.name = node.name;
                const FunctionDeclStmt* initializer = nullptr;
                for (const auto& field : node.fields)
                    klass.fields.push_back({field.name, field.fixed_type, field.is_constant});
                for (const auto& method : node.methods) {
                    if (method.name == "init" && !node.fields.empty()) {
                        initializer = &method;
                        continue;
                    }
                    std::shared_ptr<FunctionPrototype> nested;
                    compile_function(method, statement.span, nested);
                    klass.methods.emplace_back(method.name, std::move(nested));
                }
                if (!node.fields.empty()) {
                    auto nested = std::make_shared<FunctionPrototype>();
                    nested->name = "init";
                    if (initializer) nested->parameters = initializer->parameters;
                    nested->declaration_span = statement.span;
                    FunctionCompiler nested_compiler(nested);
                    klass.methods.emplace_back("init", nested_compiler.compile_initializer(
                        node.fields, initializer, statement.span));
                }
                function_->chunk.classes.push_back(std::move(klass));
                emit(OpCode::DefineClass, static_cast<int>(function_->chunk.classes.size() - 1), statement.span);
            },
            [&](const ImportStmt&) {},
            [&](const ExportStmt&) {}
        }, statement.node);
    }

    void patch_jump(int instruction_index) {
        function_->chunk.code[static_cast<std::size_t>(instruction_index)].operand =
            static_cast<int>(function_->chunk.code.size());
    }

    void compile_expression(const Expr& expression) {
        std::visit(Overloaded{
            [&](const LiteralExpr& node) {
                if (std::holds_alternative<std::monostate>(node.value)) emit(OpCode::Null, 0, expression.span);
                else if (const auto* value = std::get_if<bool>(&node.value)) emit(*value ? OpCode::True : OpCode::False, 0, expression.span);
                else if (const auto* value = std::get_if<double>(&node.value)) emit(OpCode::Constant, add_constant(*value), expression.span);
                else emit(OpCode::Constant, add_constant(std::get<std::string>(node.value)), expression.span);
            },
            [&](const VariableExpr& node) { emit(OpCode::GetName, add_constant(node.name), expression.span); },
            [&](const AssignExpr& node) {
                compile_expression(*node.value);
                emit(OpCode::SetName, add_constant(node.name), expression.span);
            },
            [&](const UnaryExpr& node) {
                compile_expression(*node.operand);
                const OpCode op = node.op == UnaryOperator::Negate ? OpCode::Negate :
                    node.op == UnaryOperator::BitNot ? OpCode::BitNot : OpCode::Not;
                emit(op, 0, expression.span);
            },
            [&](const BinaryExpr& node) {
                if (node.op == BinaryOperator::LogicalAnd) {
                    compile_expression(*node.left);
                    const int false_jump = static_cast<int>(function_->chunk.code.size());
                    emit(OpCode::JumpIfFalse, -1, node.left->span);
                    emit(OpCode::Pop, 0, node.left->span);
                    compile_expression(*node.right);
                    emit(OpCode::ToBoolean, 0, node.right->span);
                    const int end_jump = static_cast<int>(function_->chunk.code.size());
                    emit(OpCode::Jump, -1, expression.span);
                    patch_jump(false_jump);
                    emit(OpCode::Pop, 0, node.left->span);
                    emit(OpCode::False, 0, expression.span);
                    patch_jump(end_jump);
                    return;
                }
                if (node.op == BinaryOperator::LogicalOr) {
                    compile_expression(*node.left);
                    const int rhs_jump = static_cast<int>(function_->chunk.code.size());
                    emit(OpCode::JumpIfFalse, -1, node.left->span);
                    emit(OpCode::Pop, 0, node.left->span);
                    emit(OpCode::True, 0, expression.span);
                    const int end_jump = static_cast<int>(function_->chunk.code.size());
                    emit(OpCode::Jump, -1, expression.span);
                    patch_jump(rhs_jump);
                    emit(OpCode::Pop, 0, node.left->span);
                    compile_expression(*node.right);
                    emit(OpCode::ToBoolean, 0, node.right->span);
                    patch_jump(end_jump);
                    return;
                }
                compile_expression(*node.left);
                compile_expression(*node.right);
                OpCode op = OpCode::Add;
                switch (node.op) {
                    case BinaryOperator::Add: op = OpCode::Add; break;
                    case BinaryOperator::Subtract: op = OpCode::Subtract; break;
                    case BinaryOperator::Multiply: op = OpCode::Multiply; break;
                    case BinaryOperator::Divide: op = OpCode::Divide; break;
                    case BinaryOperator::FloorDivide: op = OpCode::FloorDivide; break;
                    case BinaryOperator::Modulo: op = OpCode::Modulo; break;
                    case BinaryOperator::LogicalAnd: op = OpCode::LogicalAnd; break;
                    case BinaryOperator::LogicalOr: op = OpCode::LogicalOr; break;
                    case BinaryOperator::BitwiseAnd: op = OpCode::BitwiseAnd; break;
                    case BinaryOperator::BitwiseOr: op = OpCode::BitwiseOr; break;
                    case BinaryOperator::BitwiseXor: op = OpCode::BitwiseXor; break;
                    case BinaryOperator::ShiftLeft: op = OpCode::ShiftLeft; break;
                    case BinaryOperator::ShiftRight: op = OpCode::ShiftRight; break;
                    case BinaryOperator::Equal: op = OpCode::Equal; break;
                    case BinaryOperator::NotEqual: op = OpCode::NotEqual; break;
                    case BinaryOperator::Less: op = OpCode::Less; break;
                    case BinaryOperator::LessEqual: op = OpCode::LessEqual; break;
                    case BinaryOperator::Greater: op = OpCode::Greater; break;
                    case BinaryOperator::GreaterEqual: op = OpCode::GreaterEqual; break;
                }
                emit(op, 0, expression.span);
            },
            [&](const TernaryExpr& node) {
                compile_expression(*node.condition);
                const int false_jump = static_cast<int>(function_->chunk.code.size());
                emit(OpCode::JumpIfFalse, -1, node.condition->span);
                emit(OpCode::Pop, 0, node.condition->span);
                compile_expression(*node.when_true);
                const int end_jump = static_cast<int>(function_->chunk.code.size());
                emit(OpCode::Jump, -1, expression.span);
                patch_jump(false_jump);
                emit(OpCode::Pop, 0, node.condition->span);
                compile_expression(*node.when_false);
                patch_jump(end_jump);
            },
            [&](const CallExpr& node) {
                compile_expression(*node.callee);
                for (const auto& argument : node.arguments) compile_expression(*argument);
                emit(OpCode::Call, static_cast<int>(node.arguments.size()), expression.span);
            },
            [&](const GetPropertyExpr& node) {
                compile_expression(*node.object);
                emit(OpCode::GetProperty, add_constant(node.name), expression.span);
            },
            [&](const CastExpr& node) {
                compile_expression(*node.value);
                emit(OpCode::Cast, add_constant(node.type_name), expression.span);
            },
            [&](const SetPropertyExpr& node) {
                compile_expression(*node.object);
                compile_expression(*node.value);
                emit(OpCode::SetProperty, add_constant(node.name), expression.span);
            },
            [&](const LambdaExpr& node) {
                auto nested = std::make_shared<FunctionPrototype>();
                nested->name = "<lambda>";
                nested->parameters = node.parameters;
                nested->declaration_span = expression.span;
                FunctionCompiler nested_compiler(nested);
                auto compiled = nested_compiler.compile(node.body, expression.span);
                emit(OpCode::MakeClosure, add_function(std::move(compiled)), expression.span);
            },
            [&](const IndexExpr& node) {
                compile_expression(*node.object);
                compile_expression(*node.index);
                emit(OpCode::GetIndex, 0, expression.span);
            },
            [&](const AssignIndexExpr& node) {
                compile_expression(*node.object);
                compile_expression(*node.index);
                compile_expression(*node.value);
                emit(OpCode::SetIndex, 0, expression.span);
            },
            [&](const CompoundAssignIndexExpr& node) {
                compile_expression(*node.object);
                compile_expression(*node.index);
                emit(OpCode::DuplicatePair, 0, expression.span);
                emit(OpCode::GetIndex, 0, expression.span);
                compile_expression(*node.value);
                emit(node.op == BinaryOperator::Add ? OpCode::Add : OpCode::Subtract, 0, expression.span);
                emit(OpCode::SetIndex, 0, expression.span);
            },
            [&](const CompoundAssignPropertyExpr& node) {
                compile_expression(*node.object);
                emit(OpCode::Duplicate, 0, expression.span);
                emit(OpCode::GetProperty, add_constant(node.name), expression.span);
                compile_expression(*node.value);
                emit(node.op == BinaryOperator::Add ? OpCode::Add : OpCode::Subtract, 0, expression.span);
                emit(OpCode::SetProperty, add_constant(node.name), expression.span);
            }
        }, expression.node);
    }

    std::shared_ptr<FunctionPrototype> function_;
    std::vector<LoopContext> loops_;
    int scope_depth_ = 0;
    int exception_depth_ = 0;
    int next_exception_id_ = 0;
    std::vector<const Stmt*> active_finalizers_;
};

}  // namespace

CompiledModule Compiler::compile(const Program& program) {
    CompiledModule module;
    if (!program.echo_last_expression) {
        module.diagnostics = Resolver().resolve(program);
        if (!module.diagnostics.empty()) return module;
    }
    auto entry = std::make_shared<FunctionPrototype>();
    entry->name = "<main>";
    FunctionCompiler compiler(entry);
    module.entry = compiler.compile_program(program);
    return module;
}

}  // namespace novi
