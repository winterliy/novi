#include "vm.hpp"

#include <cmath>
#include <ostream>
#include <sstream>

namespace novi {
namespace {

bool is_number(const Value& value) { return std::holds_alternative<double>(value.data); }
bool is_string(const Value& value) { return std::holds_alternative<std::string>(value.data); }

}  // namespace

VM::VM(std::ostream& output) : output_(output) {
    globals_ = make_environment();
    define_native("print", 1, [this](VM&, const std::vector<Value>& arguments, const SourceSpan&) {
        output_ << value_to_string(arguments.front()) << '\n';
        return Value{};
    });
}

std::shared_ptr<Environment> VM::make_environment(std::shared_ptr<Environment> parent) {
    return std::make_shared<Environment>(std::move(parent));
}

void VM::retain_closure_environment(const std::shared_ptr<Environment>& environment) {
    if (environment && retained_environment_ids_.insert(environment.get()).second) {
        environments_.push_back(environment);
    }
}

void VM::define_native(std::string name, std::size_t arity, NativeFunction::Callback callback) {
    auto native = std::make_shared<NativeFunction>();
    native->name = name;
    native->arity = arity;
    native->callback = std::move(callback);
    globals_->define(name, Value(native));
}

Value VM::run(const CompiledModule& module) {
    if (!module.diagnostics.empty()) throw RuntimeError("Cannot execute a module with diagnostics", {});
    if (!module.entry) throw RuntimeError("Module has no entry function", {});

    stack_.clear();
    frames_.clear();
    finished_ = false;
    final_result_ = Value{};
    auto entry = std::make_shared<FunctionObject>();
    entry->prototype = module.entry;
    entry->closure = globals_;
    push_frame(entry, {}, module.entry->declaration_span, 0);
    try {
        return execute();
    } catch (RuntimeError& error) {
        if (error.trace.empty()) {
            for (auto frame = frames_.rbegin(); frame != frames_.rend(); ++frame) {
                error.trace.push_back(frame->call_span);
            }
        }
        throw;
    }
}

void VM::push_frame(const std::shared_ptr<FunctionObject>& function,
                    const std::vector<Value>& arguments, const SourceSpan& span,
                    std::size_t stack_base, std::shared_ptr<InstanceObject> receiver,
                    std::shared_ptr<InstanceObject> return_override) {
    if (arguments.size() != function->prototype->parameters.size()) {
        fail("Function '" + function->prototype->name + "' expects " +
             std::to_string(function->prototype->parameters.size()) + " argument(s), got " +
             std::to_string(arguments.size()), span);
    }
    if (frames_.size() >= 1024) fail("Call stack limit exceeded", span);
    auto closure = function->closure.lock();
    if (!closure) fail("Function closure is no longer available", span);
    auto environment = make_environment(std::move(closure));
    if (receiver) environment->define("this", Value(receiver));
    for (std::size_t i = 0; i < arguments.size(); ++i) {
        environment->define(function->prototype->parameters[i], arguments[i]);
    }
    frames_.push_back({function, 0, std::move(environment), stack_base, span, std::move(return_override)});
}

void VM::call_value(Value callee, std::vector<Value> arguments, const SourceSpan& span) {
    const std::size_t base = stack_.size();
    if (const auto* native = std::get_if<std::shared_ptr<NativeFunction>>(&callee.data)) {
        if (arguments.size() != (*native)->arity) {
            fail("Native function '" + (*native)->name + "' expects " + std::to_string((*native)->arity) +
                 " argument(s), got " + std::to_string(arguments.size()), span);
        }
        try {
            stack_.push_back((*native)->callback(*this, arguments, span));
        } catch (const RuntimeError&) {
            throw;
        } catch (const std::exception& error) {
            fail(error.what(), span);
        }
        return;
    }
    if (const auto* function = std::get_if<std::shared_ptr<FunctionObject>>(&callee.data)) {
        push_frame(*function, arguments, span, base);
        return;
    }
    if (const auto* bound = std::get_if<std::shared_ptr<BoundMethod>>(&callee.data)) {
        push_frame((*bound)->function, arguments, span, base, (*bound)->receiver);
        return;
    }
    if (const auto* klass = std::get_if<std::shared_ptr<ClassObject>>(&callee.data)) {
        auto instance = std::make_shared<InstanceObject>(*klass);
        auto initializer = (*klass)->find_method("init");
        if (!initializer) {
            if (!arguments.empty()) fail("Class '" + (*klass)->name + "' has no initializer", span);
            stack_.push_back(Value(instance));
            return;
        }
        push_frame(initializer, arguments, span, base, instance, instance);
        return;
    }
    fail("Value is not callable", span);
}

Value VM::pop(const SourceSpan& span) {
    if (stack_.empty()) fail("Operand stack underflow", span);
    Value value = std::move(stack_.back());
    stack_.pop_back();
    return value;
}

const Constant& VM::constant_at(const Frame& frame, int index, const SourceSpan& span) const {
    const auto& constants = frame.function->prototype->chunk.constants;
    if (index < 0 || static_cast<std::size_t>(index) >= constants.size()) fail("Invalid constant index", span);
    return constants[static_cast<std::size_t>(index)];
}

std::string VM::name_at(const Frame& frame, int index, const SourceSpan& span) const {
    const auto* name = std::get_if<std::string>(&constant_at(frame, index, span));
    if (!name) fail("Expected a name constant", span);
    return *name;
}

[[noreturn]] void VM::fail(const std::string& message, const SourceSpan& span) const {
    std::vector<SourceSpan> trace;
    for (auto frame = frames_.rbegin(); frame != frames_.rend(); ++frame) trace.push_back(frame->call_span);
    throw RuntimeError(message, span, std::move(trace));
}

Value VM::execute() {
    while (!finished_) {
        if (frames_.empty()) fail("VM has no active call frame", {});
        Frame& frame = frames_.back();
        const auto& code = frame.function->prototype->chunk.code;
        if (frame.instruction_pointer >= code.size()) fail("Instruction pointer out of bounds", frame.call_span);
        const Instruction instruction = code[frame.instruction_pointer++];
        auto require_number = [&](const Value& value) -> double {
            if (!is_number(value)) fail("Expected a number, got " + value_to_string(value), instruction.span);
            return std::get<double>(value.data);
        };

        switch (instruction.op) {
            case OpCode::Constant: {
                const Constant& constant = constant_at(frame, instruction.operand, instruction.span);
                if (std::holds_alternative<std::monostate>(constant)) stack_.emplace_back();
                else if (const auto* boolean = std::get_if<bool>(&constant)) stack_.emplace_back(*boolean);
                else if (const auto* number = std::get_if<double>(&constant)) stack_.emplace_back(*number);
                else stack_.emplace_back(std::get<std::string>(constant));
                break;
            }
            case OpCode::Null: stack_.emplace_back(); break;
            case OpCode::True: stack_.emplace_back(true); break;
            case OpCode::False: stack_.emplace_back(false); break;
            case OpCode::Pop: pop(instruction.span); break;
            case OpCode::GetName: stack_.push_back(frame.environment->get(name_at(frame, instruction.operand, instruction.span), instruction.span)); break;
            case OpCode::DefineName: frame.environment->define(name_at(frame, instruction.operand, instruction.span), pop(instruction.span)); break;
            case OpCode::SetName: {
                if (stack_.empty()) fail("Operand stack underflow", instruction.span);
                const std::string name = name_at(frame, instruction.operand, instruction.span);
                frame.environment->assign(name, stack_.back(), instruction.span);
                break;
            }
            case OpCode::Negate: {
                const double number = require_number(pop(instruction.span));
                stack_.emplace_back(-number);
                break;
            }
            case OpCode::Not: stack_.emplace_back(!value_is_truthy(pop(instruction.span))); break;
            case OpCode::Add: {
                Value right = pop(instruction.span);
                Value left = pop(instruction.span);
                if (is_number(left) && is_number(right)) stack_.emplace_back(std::get<double>(left.data) + std::get<double>(right.data));
                else if (is_string(left) && is_string(right)) stack_.emplace_back(std::get<std::string>(left.data) + std::get<std::string>(right.data));
                else fail("'+' expects two numbers or two strings", instruction.span);
                break;
            }
            case OpCode::Subtract: case OpCode::Multiply: case OpCode::Divide:
            case OpCode::Less: case OpCode::LessEqual: case OpCode::Greater: case OpCode::GreaterEqual: {
                const double right = require_number(pop(instruction.span));
                const double left = require_number(pop(instruction.span));
                if (instruction.op == OpCode::Subtract) stack_.emplace_back(left - right);
                else if (instruction.op == OpCode::Multiply) stack_.emplace_back(left * right);
                else if (instruction.op == OpCode::Divide) {
                    if (right == 0.0) fail("Division by zero", instruction.span);
                    stack_.emplace_back(left / right);
                } else if (instruction.op == OpCode::Less) stack_.emplace_back(left < right);
                else if (instruction.op == OpCode::LessEqual) stack_.emplace_back(left <= right);
                else if (instruction.op == OpCode::Greater) stack_.emplace_back(left > right);
                else stack_.emplace_back(left >= right);
                break;
            }
            case OpCode::Equal: case OpCode::NotEqual: {
                const Value right = pop(instruction.span);
                const Value left = pop(instruction.span);
                const bool equal = values_equal(left, right);
                stack_.emplace_back(instruction.op == OpCode::Equal ? equal : !equal);
                break;
            }
            case OpCode::Jump:
                if (instruction.operand < 0 || static_cast<std::size_t>(instruction.operand) > code.size()) fail("Invalid jump target", instruction.span);
                frame.instruction_pointer = static_cast<std::size_t>(instruction.operand);
                break;
            case OpCode::JumpIfFalse:
                if (stack_.empty()) fail("Operand stack underflow", instruction.span);
                if (!value_is_truthy(stack_.back())) {
                    if (instruction.operand < 0 || static_cast<std::size_t>(instruction.operand) > code.size()) fail("Invalid jump target", instruction.span);
                    frame.instruction_pointer = static_cast<std::size_t>(instruction.operand);
                }
                break;
            case OpCode::EnterScope: frame.environment = make_environment(frame.environment); break;
            case OpCode::ExitScope:
                if (!frame.environment->parent) fail("Cannot exit the global scope", instruction.span);
                frame.environment = frame.environment->parent;
                break;
            case OpCode::MakeClosure: {
                const auto& functions = frame.function->prototype->chunk.functions;
                if (instruction.operand < 0 || static_cast<std::size_t>(instruction.operand) >= functions.size()) fail("Invalid function index", instruction.span);
                auto function = std::make_shared<FunctionObject>();
                function->prototype = functions[static_cast<std::size_t>(instruction.operand)];
                function->closure = frame.environment;
                retain_closure_environment(frame.environment);
                stack_.emplace_back(function);
                break;
            }
            case OpCode::DefineClass: {
                Value superclass_value = pop(instruction.span);
                std::shared_ptr<ClassObject> superclass;
                if (!std::holds_alternative<std::monostate>(superclass_value.data)) {
                    const auto* value = std::get_if<std::shared_ptr<ClassObject>>(&superclass_value.data);
                    if (!value) fail("Superclass must be a class", instruction.span);
                    superclass = *value;
                }
                const auto& classes = frame.function->prototype->chunk.classes;
                if (instruction.operand < 0 || static_cast<std::size_t>(instruction.operand) >= classes.size()) fail("Invalid class index", instruction.span);
                const ClassPrototype& prototype = classes[static_cast<std::size_t>(instruction.operand)];
                auto klass = std::make_shared<ClassObject>();
                klass->name = prototype.name;
                klass->superclass = std::move(superclass);
                for (const auto& method : prototype.methods) {
                    auto function = std::make_shared<FunctionObject>();
                    function->prototype = method.second;
                    function->closure = frame.environment;
                    retain_closure_environment(frame.environment);
                    klass->methods[method.first] = std::move(function);
                }
                frame.environment->define(klass->name, Value(klass));
                break;
            }
            case OpCode::GetProperty: {
                Value object = pop(instruction.span);
                const auto* instance_value = std::get_if<std::shared_ptr<InstanceObject>>(&object.data);
                if (!instance_value) fail("Only instances have properties", instruction.span);
                const std::string name = name_at(frame, instruction.operand, instruction.span);
                const auto field = (*instance_value)->fields.find(name);
                if (field != (*instance_value)->fields.end()) stack_.push_back(field->second);
                else if (auto method = (*instance_value)->klass->find_method(name)) {
                    auto bound = std::make_shared<BoundMethod>();
                    bound->function = std::move(method);
                    bound->receiver = *instance_value;
                    stack_.emplace_back(std::move(bound));
                } else fail("Undefined property '" + name + "'", instruction.span);
                break;
            }
            case OpCode::SetProperty: {
                Value value = pop(instruction.span);
                Value object = pop(instruction.span);
                const auto* instance_value = std::get_if<std::shared_ptr<InstanceObject>>(&object.data);
                if (!instance_value) fail("Only instances have fields", instruction.span);
                const std::string name = name_at(frame, instruction.operand, instruction.span);
                (*instance_value)->fields[name] = value;
                stack_.push_back(std::move(value));
                break;
            }
            case OpCode::Call: {
                if (instruction.operand < 0 || stack_.size() < static_cast<std::size_t>(instruction.operand + 1)) fail("Invalid call stack layout", instruction.span);
                std::vector<Value> arguments(static_cast<std::size_t>(instruction.operand));
                for (int i = instruction.operand - 1; i >= 0; --i) arguments[static_cast<std::size_t>(i)] = pop(instruction.span);
                Value callee = pop(instruction.span);
                call_value(std::move(callee), std::move(arguments), instruction.span);
                break;
            }
            case OpCode::Return: {
                Value result = pop(instruction.span);
                Frame finished = std::move(frames_.back());
                frames_.pop_back();
                if (finished.return_override) result = Value(finished.return_override);
                stack_.resize(finished.stack_base);
                if (frames_.empty()) {
                    final_result_ = std::move(result);
                    finished_ = true;
                } else stack_.push_back(std::move(result));
                break;
            }
        }
    }
    return final_result_;
}

}  // namespace novi
