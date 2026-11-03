#pragma once

#include <iosfwd>
#include <memory>
#include <unordered_set>
#include <vector>

#include "bytecode.hpp"
#include "value.hpp"

namespace novi {

class VM {
public:
    explicit VM(std::ostream& output);
    Value run(const CompiledModule& module);
    void define_native(std::string name, std::size_t arity, NativeFunction::Callback callback);

private:
    struct Frame {
        std::shared_ptr<FunctionObject> function;
        std::size_t instruction_pointer = 0;
        std::shared_ptr<Environment> environment;
        std::size_t stack_base = 0;
        SourceSpan call_span;
        std::shared_ptr<InstanceObject> return_override;
    };

    Value execute();
    std::shared_ptr<Environment> make_environment(std::shared_ptr<Environment> parent = nullptr);
    void retain_closure_environment(const std::shared_ptr<Environment>& environment);
    void call_value(Value callee, std::vector<Value> arguments, const SourceSpan& span);
    void push_frame(const std::shared_ptr<FunctionObject>& function,
                    const std::vector<Value>& arguments, const SourceSpan& span,
                    std::size_t stack_base, std::shared_ptr<InstanceObject> receiver = nullptr,
                    std::shared_ptr<InstanceObject> return_override = nullptr);
    Value pop(const SourceSpan& span);
    const Constant& constant_at(const Frame& frame, int index, const SourceSpan& span) const;
    std::string name_at(const Frame& frame, int index, const SourceSpan& span) const;
    [[noreturn]] void fail(const std::string& message, const SourceSpan& span) const;

    std::ostream& output_;
    std::shared_ptr<Environment> globals_;
    std::vector<std::shared_ptr<Environment>> environments_;
    std::unordered_set<const Environment*> retained_environment_ids_;
    std::vector<Value> stack_;
    std::vector<Frame> frames_;
    bool finished_ = false;
    Value final_result_;
};

}  // namespace novi
