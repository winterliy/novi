#include "vm.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <optional>
#include <ostream>
#include <sstream>

namespace novi {
namespace {

bool is_number(const Value& value) { return std::holds_alternative<double>(value.data); }
bool is_string(const Value& value) { return std::holds_alternative<std::string>(value.data); }

std::optional<std::string> string_character_at(const std::string& value, std::size_t target) {
    std::size_t offset = 0;
    std::size_t character = 0;
    while (offset < value.size()) {
        const std::size_t start = offset;
        const auto lead = static_cast<unsigned char>(value[offset]);
        const std::size_t width = lead < 0x80 ? 1 : lead < 0xE0 ? 2 : lead < 0xF0 ? 3 : 4;
        offset = std::min(value.size(), offset + width);
        if (character++ == target) return value.substr(start, offset - start);
    }
    return std::nullopt;
}

std::size_t string_codepoint_count(const std::string& value) {
    std::size_t offset = 0;
    std::size_t count = 0;
    while (offset < value.size()) {
        const auto lead = static_cast<unsigned char>(value[offset]);
        const std::size_t width = lead < 0x80 ? 1 : lead < 0xE0 ? 2 : lead < 0xF0 ? 3 : 4;
        offset = std::min(value.size(), offset + width);
        ++count;
    }
    return count;
}

}  // namespace

VM::VM(std::ostream& output) : output_(output) {
    globals_ = make_environment();
    define_native("print", 1, [](VM&, const std::vector<Value>& arguments, const SourceSpan&) {
        output_ << value_to_string(arguments.front()) << '\n';
        return Value{};
    });
    define_native("mod", 2, [](VM&, const std::vector<Value>& arguments, const SourceSpan&) {
        const auto* left = std::get_if<double>(&arguments[0].data);
        const auto* right = std::get_if<double>(&arguments[1].data);
        if (!left || !right) throw std::runtime_error("mod(a, b) expects numeric arguments");
        if (*right == 0.0) throw std::runtime_error("Modulo by zero");
        return Value(std::fmod(*left, *right));
    });
    define_native("istrue", 1, [](VM&, const std::vector<Value>& arguments, const SourceSpan&) {
        return Value(value_is_truthy(arguments.front()));
    });
    define_native("string", 1, [](VM&, const std::vector<Value>& arguments, const SourceSpan&) {
        return Value(value_to_string(arguments.front()));
    });
    define_native("integer", 1, [](VM&, const std::vector<Value>& arguments, const SourceSpan&) {
        const auto* number = std::get_if<double>(&arguments.front().data);
        if (!number) throw std::runtime_error("integer(value) expects a number");
        return Value(std::trunc(*number));
    });
    define_native("range", 3, [](VM&, const std::vector<Value>& arguments, const SourceSpan&) {
        constexpr double max_exact_integer = 9007199254740991.0;
        auto as_integer = [&](const Value& value, const char* parameter) -> std::int64_t {
            const auto* number = std::get_if<double>(&value.data);
            if (!number || !std::isfinite(*number) || std::trunc(*number) != *number ||
                std::abs(*number) > max_exact_integer) {
                throw std::runtime_error(std::string("range ") + parameter +
                    " must be an exactly represented integer within the safe integer range");
            }
            return static_cast<std::int64_t>(*number);
        };
        const auto start = as_integer(arguments[0], "start");
        const auto step = as_integer(arguments[1], "step");
        const auto stop = as_integer(arguments[2], "stop");
        if (step == 0) throw std::runtime_error("range step must not be zero");
        auto range = std::make_shared<RangeObject>();
        range->start = start;
        range->step = step;
        range->stop = stop;
        return Value(std::move(range));
    });
    define_native("dictionary", 0, [](VM&, const std::vector<Value>&, const SourceSpan&) {
        return Value(std::make_shared<DictionaryObject>());
    });
    define_native("list", 0, [](VM&, const std::vector<Value>&, const SourceSpan&) {
        return Value(std::make_shared<ListObject>());
    });
    define_native("set", 0, [](VM&, const std::vector<Value>&, const SourceSpan&) {
        return Value(std::make_shared<SetObject>());
    });
    define_native("queue", 0, [](VM&, const std::vector<Value>&, const SourceSpan&) {
        return Value(std::make_shared<QueueObject>());
    });
    auto tuple_native = std::make_shared<NativeFunction>();
    tuple_native->name = "tuple";
    tuple_native->variadic = true;
    tuple_native->callback = [](VM&, const std::vector<Value>& arguments, const SourceSpan&) {
        auto tuple = std::make_shared<TupleObject>();
        tuple->values = arguments;
        return Value(std::move(tuple));
    };
    globals_->define("tuple", Value(std::move(tuple_native)));
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
    exception_handlers_.clear();
    pending_exceptions_.clear();
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
        if (!(*native)->variadic && arguments.size() != (*native)->arity) {
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

        try {
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
            case OpCode::ToBoolean: stack_.emplace_back(value_is_truthy(pop(instruction.span))); break;
            case OpCode::GetName: stack_.push_back(frame.environment->get(name_at(frame, instruction.operand, instruction.span), instruction.span)); break;
            case OpCode::DefineName: frame.environment->define(name_at(frame, instruction.operand, instruction.span), pop(instruction.span)); break;
            case OpCode::DefineConstantName: frame.environment->define_constant(name_at(frame, instruction.operand, instruction.span), pop(instruction.span)); break;
            case OpCode::DefineTypedName: case OpCode::DefineTypedConstantName: {
                Value value = pop(instruction.span);
                const std::string name = name_at(frame, instruction.operand, instruction.span);
                const std::string type = name_at(frame, instruction.operand2, instruction.span);
                frame.environment->define_typed(name, std::move(value), type,
                    instruction.op == OpCode::DefineTypedConstantName, instruction.span);
                break;
            }
            case OpCode::FreezeName: frame.environment->make_constant(name_at(frame, instruction.operand, instruction.span), instruction.span); break;
            case OpCode::Cast: {
                Value value = pop(instruction.span);
                const std::string type = name_at(frame, instruction.operand, instruction.span);
                if (type == "string") stack_.emplace_back(value_to_string(value));
                else if (type == "integer") {
                    const auto* number = std::get_if<double>(&value.data);
                    if (!number) fail("Cannot cast " + value_to_string(value) + " to integer", instruction.span);
                    stack_.emplace_back(std::trunc(*number));
                } else if (type == "boolean") stack_.emplace_back(value_is_truthy(value));
                else fail("Unknown output cast type '" + type + "'", instruction.span);
                break;
            }
            case OpCode::IteratorInit: {
                Value iterable = pop(instruction.span);
                if (const auto* range = std::get_if<std::shared_ptr<RangeObject>>(&iterable.data)) {
                    stack_.emplace_back(std::make_shared<IteratorObject>(*range));
                } else if (const auto* dictionary = std::get_if<std::shared_ptr<DictionaryObject>>(&iterable.data)) {
                    auto iterator = std::make_shared<DictionaryIteratorObject>(*dictionary);
                    iterator->snapshot = (*dictionary)->entries;
                    stack_.emplace_back(std::move(iterator));
                } else if (const auto* list = std::get_if<std::shared_ptr<ListObject>>(&iterable.data)) {
                    if (!*list) fail("Invalid list", instruction.span);
                    stack_.emplace_back(std::make_shared<SequenceIteratorObject>((*list)->values));
                } else if (const auto* set = std::get_if<std::shared_ptr<SetObject>>(&iterable.data)) {
                    if (!*set) fail("Invalid set", instruction.span);
                    stack_.emplace_back(std::make_shared<SequenceIteratorObject>((*set)->values));
                } else if (const auto* queue = std::get_if<std::shared_ptr<QueueObject>>(&iterable.data)) {
                    if (!*queue) fail("Invalid queue", instruction.span);
                    stack_.emplace_back(std::make_shared<SequenceIteratorObject>(
                        std::vector<Value>((*queue)->values.begin(), (*queue)->values.end())));
                } else if (const auto* tuple = std::get_if<std::shared_ptr<TupleObject>>(&iterable.data)) {
                    if (!*tuple) fail("Invalid tuple", instruction.span);
                    stack_.emplace_back(std::make_shared<SequenceIteratorObject>((*tuple)->values));
                } else fail("foreach expects a range, list, tuple, set, queue, or dictionary", instruction.span);
                break;
            }
            case OpCode::IteratorNext: {
                if (stack_.empty()) fail("Iterator stack underflow", instruction.span);
                const std::string variable = name_at(frame, instruction.operand, instruction.span);
                auto bind = [&](const std::string& name, const Value& value) {
                    if (frame.environment->contains_local(name)) frame.environment->assign(name, value, instruction.span);
                    else frame.environment->define(name, value);
                };
                bool has_next = false;
                const int exit_target = instruction.operand2;
                if (const auto* iterator = std::get_if<std::shared_ptr<IteratorObject>>(&stack_.back().data)) {
                    if (!*iterator) fail("Invalid foreach iterator", instruction.span);
                    if (instruction.operand3 >= 0)
                        fail("A range yields one value; use one foreach variable", instruction.span);
                    const auto& range = *(*iterator)->range;
                    has_next = range.step > 0 ? (*iterator)->current < range.stop
                                              : (*iterator)->current > range.stop;
                    if (has_next) {
                        bind(variable, Value(static_cast<double>((*iterator)->current)));
                        const auto current = (*iterator)->current;
                        if ((range.step > 0 && current > INT64_MAX - range.step) ||
                            (range.step < 0 && current < INT64_MIN - range.step))
                            fail("range iteration overflow", instruction.span);
                        (*iterator)->current += range.step;
                    }
                } else if (const auto* iterator = std::get_if<std::shared_ptr<DictionaryIteratorObject>>(&stack_.back().data)) {
                    if (!*iterator) fail("Invalid dictionary iterator", instruction.span);
                    if ((*iterator)->mode == DictionaryIteratorObject::Projection::Values && instruction.operand3 >= 0)
                        fail("A list yields one value; use one foreach variable", instruction.span);
                    has_next = (*iterator)->index < (*iterator)->snapshot.size();
                    if (has_next) {
                        const auto& entry = (*iterator)->snapshot[(*iterator)->index++];
                        const Value& first_value = (*iterator)->mode == DictionaryIteratorObject::Projection::Values
                            ? entry.second : entry.first;
                        bind(variable, first_value);
                        if (instruction.operand3 >= 0)
                            bind(name_at(frame, instruction.operand3, instruction.span), entry.second);
                    }
                } else if (const auto* iterator = std::get_if<std::shared_ptr<SequenceIteratorObject>>(&stack_.back().data)) {
                    if (!*iterator) fail("Invalid collection iterator", instruction.span);
                    if (instruction.operand3 >= 0)
                        fail("This collection yields one value; use one foreach variable", instruction.span);
                    has_next = (*iterator)->index < (*iterator)->values.size();
                    if (has_next) bind(variable, (*iterator)->values[(*iterator)->index++]);
                } else fail("Invalid foreach iterator", instruction.span);
                if (!has_next) {
                    if (exit_target < 0 || static_cast<std::size_t>(exit_target) > code.size())
                        fail("Invalid foreach exit target", instruction.span);
                    frame.instruction_pointer = static_cast<std::size_t>(exit_target);
                }
                break;
            }
            case OpCode::PushExceptionHandler: {
                const bool valid_catch = instruction.operand == -1 ||
                    (instruction.operand >= 0 && static_cast<std::size_t>(instruction.operand) <= code.size());
                const bool valid_finally = instruction.operand3 == -1 ||
                    (instruction.operand3 >= 0 && static_cast<std::size_t>(instruction.operand3) <= code.size());
                if (!valid_catch || !valid_finally || (instruction.operand == -1 && instruction.operand3 == -1))
                    fail("Invalid exception handler target", instruction.span);
                exception_handlers_.push_back({frames_.size() - 1, instruction.operand,
                    instruction.operand3, stack_.size(), frame.environment,
                    instruction.operand2 >= 0 ? name_at(frame, instruction.operand2, instruction.span) : "",
                    instruction.operand4, false});
                break;
            }
            case OpCode::PopExceptionHandler:
                if (exception_handlers_.empty() || exception_handlers_.back().frame_index != frames_.size() - 1)
                    fail("Exception handler stack is unbalanced", instruction.span);
                exception_handlers_.pop_back();
                break;
            case OpCode::Throw:
                fail(name_at(frame, instruction.operand, instruction.span), instruction.span);
            case OpCode::Rethrow: {
                const auto pending = std::find_if(pending_exceptions_.rbegin(), pending_exceptions_.rend(),
                    [&](const PendingException& item) {
                        return item.frame_index == frames_.size() - 1 && item.exception_id == instruction.operand;
                    });
                if (pending == pending_exceptions_.rend()) fail("No pending exception to rethrow", instruction.span);
                RuntimeError error = pending->error;
                pending_exceptions_.erase(std::next(pending).base());
                throw error;
            }
            case OpCode::GetIndex: {
                Value key = pop(instruction.span);
                Value object = pop(instruction.span);
                if (const auto* dictionary = std::get_if<std::shared_ptr<DictionaryObject>>(&object.data)) {
                    if (!*dictionary) fail("Invalid dictionary", instruction.span);
                    const auto found = (*dictionary)->get(key);
                    stack_.push_back(found ? *found : Value{});
                } else if (const auto* list = std::get_if<std::shared_ptr<ListObject>>(&object.data)) {
                    const auto* index = std::get_if<double>(&key.data);
                    if (!*list || !index || !std::isfinite(*index) || std::trunc(*index) != *index || *index < 0.0 ||
                        *index >= static_cast<double>((*list)->values.size()))
                        fail("List index must be an in-range non-negative integer", instruction.span);
                    stack_.push_back((*list)->values[static_cast<std::size_t>(*index)]);
                } else if (const auto* string = std::get_if<std::string>(&object.data)) {
                    const auto* index = std::get_if<double>(&key.data);
                    if (!index || !std::isfinite(*index) || std::trunc(*index) != *index || *index < 0.0 ||
                        *index >= static_cast<double>(string->size()))
                        fail("String index must be an in-range non-negative integer", instruction.span);
                    const auto character = string_character_at(*string, static_cast<std::size_t>(*index));
                    if (!character) fail("String index is out of range", instruction.span);
                    stack_.emplace_back(*character);
                } else if (const auto* tuple = std::get_if<std::shared_ptr<TupleObject>>(&object.data)) {
                    const auto* index = std::get_if<double>(&key.data);
                    if (!*tuple || !index || !std::isfinite(*index) || std::trunc(*index) != *index || *index < 0.0 ||
                        *index >= static_cast<double>((*tuple)->values.size()))
                        fail("Tuple index must be an in-range non-negative integer", instruction.span);
                    stack_.push_back((*tuple)->values[static_cast<std::size_t>(*index)]);
                } else fail("Indexing requires a dictionary, list, tuple, or string", instruction.span);
                break;
            }
            case OpCode::SetIndex: {
                Value value = pop(instruction.span);
                Value key = pop(instruction.span);
                Value object = pop(instruction.span);
                if (const auto* dictionary = std::get_if<std::shared_ptr<DictionaryObject>>(&object.data)) {
                    if (!*dictionary) fail("Invalid dictionary", instruction.span);
                    (*dictionary)->set(std::move(key), value);
                } else if (const auto* list = std::get_if<std::shared_ptr<ListObject>>(&object.data)) {
                    const auto* index = std::get_if<double>(&key.data);
                    if (!*list || !index || !std::isfinite(*index) || std::trunc(*index) != *index || *index < 0.0 ||
                        *index >= static_cast<double>((*list)->values.size()))
                        fail("List index must be an in-range non-negative integer", instruction.span);
                    (*list)->values[static_cast<std::size_t>(*index)] = value;
                } else if (std::holds_alternative<std::shared_ptr<TupleObject>>(object.data)) {
                    fail("Tuples are immutable", instruction.span);
                } else fail("Indexed assignment requires a dictionary or list", instruction.span);
                stack_.push_back(std::move(value));
                break;
            }
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
            case OpCode::BitNot: {
                const double number = require_number(pop(instruction.span));
                stack_.emplace_back(static_cast<double>(~static_cast<std::int64_t>(number)));
                break;
            }
            case OpCode::Add: {
                Value right = pop(instruction.span);
                Value left = pop(instruction.span);
                if (is_number(left) && is_number(right)) stack_.emplace_back(std::get<double>(left.data) + std::get<double>(right.data));
                else if (is_string(left) && is_string(right)) stack_.emplace_back(std::get<std::string>(left.data) + std::get<std::string>(right.data));
                else fail("'+' expects two numbers or two strings", instruction.span);
                break;
            }
            case OpCode::Subtract: case OpCode::Multiply: case OpCode::Divide:
            case OpCode::FloorDivide: case OpCode::Modulo:
            case OpCode::BitwiseAnd: case OpCode::BitwiseOr: case OpCode::BitwiseXor:
            case OpCode::ShiftLeft: case OpCode::ShiftRight:
            case OpCode::Less: case OpCode::LessEqual: case OpCode::Greater: case OpCode::GreaterEqual: {
                const double right = require_number(pop(instruction.span));
                const double left = require_number(pop(instruction.span));
                if (instruction.op == OpCode::Subtract) stack_.emplace_back(left - right);
                else if (instruction.op == OpCode::Multiply) stack_.emplace_back(left * right);
                else if (instruction.op == OpCode::Divide) {
                    if (right == 0.0) fail("Division by zero", instruction.span);
                    stack_.emplace_back(left / right);
                } else if (instruction.op == OpCode::FloorDivide) {
                    if (right == 0.0) fail("Division by zero", instruction.span);
                    stack_.emplace_back(std::floor(left / right));
                } else if (instruction.op == OpCode::Modulo) {
                    if (right == 0.0) fail("Modulo by zero", instruction.span);
                    stack_.emplace_back(std::fmod(left, right));
                } else if (instruction.op == OpCode::BitwiseAnd || instruction.op == OpCode::BitwiseOr ||
                           instruction.op == OpCode::BitwiseXor || instruction.op == OpCode::ShiftLeft ||
                           instruction.op == OpCode::ShiftRight) {
                    const auto lhs = static_cast<std::int64_t>(left);
                    const auto rhs = static_cast<std::int64_t>(right);
                    if ((instruction.op == OpCode::ShiftLeft || instruction.op == OpCode::ShiftRight) &&
                        (rhs < 0 || rhs >= 63)) fail("Shift count must be between 0 and 62", instruction.span);
                    if (instruction.op == OpCode::BitwiseAnd) stack_.emplace_back(static_cast<double>(lhs & rhs));
                    else if (instruction.op == OpCode::BitwiseOr) stack_.emplace_back(static_cast<double>(lhs | rhs));
                    else if (instruction.op == OpCode::BitwiseXor) stack_.emplace_back(static_cast<double>(lhs ^ rhs));
                    else if (instruction.op == OpCode::ShiftLeft) stack_.emplace_back(static_cast<double>(lhs << rhs));
                    else stack_.emplace_back(static_cast<double>(lhs >> rhs));
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
            case OpCode::LogicalAnd: case OpCode::LogicalOr: {
                const Value right = pop(instruction.span);
                const Value left = pop(instruction.span);
                const bool result = instruction.op == OpCode::LogicalAnd
                    ? value_is_truthy(left) && value_is_truthy(right)
                    : value_is_truthy(left) || value_is_truthy(right);
                stack_.emplace_back(result);
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
                if (const auto* dictionary = std::get_if<std::shared_ptr<DictionaryObject>>(&object.data)) {
                    const std::string name = name_at(frame, instruction.operand, instruction.span);
                    if (name == "keys" || name == "values") {
                        auto method = std::make_shared<NativeFunction>();
                        method->name = "dictionary." + name;
                        method->arity = 0;
                        const auto source = *dictionary;
                        method->callback = [source, name](VM&, const std::vector<Value>&, const SourceSpan&) {
                            auto list = std::make_shared<ListObject>();
                            for (const auto& entry : source->entries)
                                list->values.push_back(name == "keys" ? entry.first : entry.second);
                            return Value(std::move(list));
                        };
                        stack_.emplace_back(std::move(method));
                        break;
                    }
                    fail("Undefined dictionary method '" + name + "'", instruction.span);
                }
                if (const auto* list = std::get_if<std::shared_ptr<ListObject>>(&object.data)) {
                    const std::string name = name_at(frame, instruction.operand, instruction.span);
                    if (!*list) fail("Invalid list", instruction.span);
                    auto method = std::make_shared<NativeFunction>();
                    method->name = "list." + name;
                    const auto source = *list;
                    if (name == "append") {
                        method->arity = 1;
                        method->callback = [source](VM&, const std::vector<Value>& args, const SourceSpan&) {
                            source->values.push_back(args[0]);
                            return Value{};
                        };
                    } else if (name == "contains") {
                        method->arity = 1;
                        method->callback = [source](VM&, const std::vector<Value>& args, const SourceSpan&) {
                            return Value(std::any_of(source->values.begin(), source->values.end(),
                                [&](const Value& value) { return values_equal(value, args[0]); }));
                        };
                    } else if (name == "remove") {
                        method->arity = 1;
                        method->callback = [source](VM&, const std::vector<Value>& args, const SourceSpan&) {
                            const auto found = std::find_if(source->values.begin(), source->values.end(),
                                [&](const Value& value) { return values_equal(value, args[0]); });
                            if (found == source->values.end()) return Value(false);
                            source->values.erase(found);
                            return Value(true);
                        };
                    } else if (name == "clear") {
                        method->arity = 0;
                        method->callback = [source](VM&, const std::vector<Value>&, const SourceSpan&) {
                            source->values.clear();
                            return Value{};
                        };
                    } else if (name == "length") {
                        method->arity = 0;
                        method->callback = [source](VM&, const std::vector<Value>&, const SourceSpan&) {
                            return Value(static_cast<double>(source->values.size()));
                        };
                    } else fail("Undefined list method '" + name + "'", instruction.span);
                    stack_.emplace_back(std::move(method));
                    break;
                }
                if (const auto* set = std::get_if<std::shared_ptr<SetObject>>(&object.data)) {
                    const std::string name = name_at(frame, instruction.operand, instruction.span);
                    if (!*set) fail("Invalid set", instruction.span);
                    auto method = std::make_shared<NativeFunction>();
                    method->name = "set." + name;
                    const auto source = *set;
                    if (name == "add") {
                        method->arity = 1;
                        method->callback = [source](VM&, const std::vector<Value>& args, const SourceSpan&) {
                            if (std::none_of(source->values.begin(), source->values.end(),
                                [&](const Value& value) { return values_equal(value, args[0]); }))
                                source->values.push_back(args[0]);
                            return Value{};
                        };
                    } else if (name == "contains") {
                        method->arity = 1;
                        method->callback = [source](VM&, const std::vector<Value>& args, const SourceSpan&) {
                            return Value(std::any_of(source->values.begin(), source->values.end(),
                                [&](const Value& value) { return values_equal(value, args[0]); }));
                        };
                    } else if (name == "length") {
                        method->arity = 0;
                        method->callback = [source](VM&, const std::vector<Value>&, const SourceSpan&) {
                            return Value(static_cast<double>(source->values.size()));
                        };
                    } else fail("Undefined set method '" + name + "'", instruction.span);
                    stack_.emplace_back(std::move(method));
                    break;
                }
                if (const auto* queue = std::get_if<std::shared_ptr<QueueObject>>(&object.data)) {
                    const std::string name = name_at(frame, instruction.operand, instruction.span);
                    if (!*queue) fail("Invalid queue", instruction.span);
                    auto method = std::make_shared<NativeFunction>();
                    method->name = "queue." + name;
                    const auto source = *queue;
                    if (name == "enqueue") {
                        method->arity = 1;
                        method->callback = [source](VM&, const std::vector<Value>& args, const SourceSpan&) {
                            source->values.push_back(args[0]);
                            return Value{};
                        };
                    } else if (name == "dequeue") {
                        method->arity = 0;
                        method->callback = [source](VM&, const std::vector<Value>&, const SourceSpan&) {
                            if (source->values.empty()) throw std::runtime_error("Cannot dequeue from an empty queue");
                            Value value = std::move(source->values.front());
                            source->values.pop_front();
                            return value;
                        };
                    } else if (name == "length") {
                        method->arity = 0;
                        method->callback = [source](VM&, const std::vector<Value>&, const SourceSpan&) {
                            return Value(static_cast<double>(source->values.size()));
                        };
                    } else fail("Undefined queue method '" + name + "'", instruction.span);
                    stack_.emplace_back(std::move(method));
                    break;
                }
                if (const auto* string = std::get_if<std::string>(&object.data)) {
                    const std::string name = name_at(frame, instruction.operand, instruction.span);
                    if (name != "length") fail("Undefined string method '" + name + "'", instruction.span);
                    auto method = std::make_shared<NativeFunction>();
                    method->name = "string.length";
                    method->arity = 0;
                    const std::size_t length = string_codepoint_count(*string);
                    method->callback = [length](VM&, const std::vector<Value>&, const SourceSpan&) {
                        return Value(static_cast<double>(length));
                    };
                    stack_.emplace_back(std::move(method));
                    break;
                }
                if (const auto* tuple = std::get_if<std::shared_ptr<TupleObject>>(&object.data)) {
                    const std::string name = name_at(frame, instruction.operand, instruction.span);
                    if (!*tuple) fail("Invalid tuple", instruction.span);
                    if (name != "length") fail("Undefined tuple method '" + name + "'", instruction.span);
                    auto method = std::make_shared<NativeFunction>();
                    method->name = "tuple.length";
                    method->arity = 0;
                    const double length = static_cast<double>((*tuple)->values.size());
                    method->callback = [length](VM&, const std::vector<Value>&, const SourceSpan&) {
                        return Value(length);
                    };
                    stack_.emplace_back(std::move(method));
                    break;
                }
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
                while (!exception_handlers_.empty() &&
                       exception_handlers_.back().frame_index == frames_.size() - 1)
                    exception_handlers_.pop_back();
                pending_exceptions_.erase(std::remove_if(pending_exceptions_.begin(), pending_exceptions_.end(),
                    [&](const PendingException& item) { return item.frame_index == frames_.size() - 1; }),
                    pending_exceptions_.end());
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
        } catch (const RuntimeError& error) {
            bool handled = false;
            while (!exception_handlers_.empty()) {
                ExceptionHandler& top = exception_handlers_.back();
                if (!top.caught && top.catch_target >= 0) {
                    top.caught = true;
                    const ExceptionHandler handler = top;
                    while (frames_.size() > handler.frame_index + 1) frames_.pop_back();
                    if (handler.frame_index >= frames_.size()) throw;
                    stack_.resize(handler.stack_size);
                    Frame& catch_frame = frames_[handler.frame_index];
                    catch_frame.environment = make_environment(handler.environment);
                    catch_frame.environment->define(handler.error_name, Value(error.what()));
                    catch_frame.instruction_pointer = static_cast<std::size_t>(handler.catch_target);
                    handled = true;
                    break;
                }
                if (top.finally_target >= 0) {
                    ExceptionHandler handler = std::move(top);
                    exception_handlers_.pop_back();
                    while (frames_.size() > handler.frame_index + 1) frames_.pop_back();
                    if (handler.frame_index >= frames_.size()) throw;
                    stack_.resize(handler.stack_size);
                    Frame& finally_frame = frames_[handler.frame_index];
                    finally_frame.environment = std::move(handler.environment);
                    pending_exceptions_.erase(std::remove_if(pending_exceptions_.begin(), pending_exceptions_.end(),
                        [&](const PendingException& item) {
                            return item.frame_index == handler.frame_index && item.exception_id == handler.exception_id;
                        }), pending_exceptions_.end());
                    pending_exceptions_.push_back({handler.frame_index, handler.exception_id, error});
                    finally_frame.instruction_pointer = static_cast<std::size_t>(handler.finally_target);
                    handled = true;
                    break;
                }
                exception_handlers_.pop_back();
            }
            if (!handled) throw;
        }
    }
    return final_result_;
}

}  // namespace novi
