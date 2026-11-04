#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "bytecode.hpp"

namespace novi {

class VM;
struct Environment;
struct FunctionObject;
struct ClassObject;
struct InstanceObject;
struct BoundMethod;
struct NativeFunction;
struct RangeObject;
struct IteratorObject;
struct ListObject;
struct DictionaryObject;
struct DictionaryIteratorObject;

class RuntimeError : public std::runtime_error {
public:
    RuntimeError(std::string message, SourceSpan error_span, std::vector<SourceSpan> frames = {})
        : std::runtime_error(std::move(message)), span(std::move(error_span)), trace(std::move(frames)) {}

    SourceSpan span;
    std::vector<SourceSpan> trace;
};

struct Value {
    using Storage = std::variant<std::monostate, bool, double, std::string,
                                 std::shared_ptr<NativeFunction>, std::shared_ptr<FunctionObject>,
                                 std::shared_ptr<ClassObject>, std::shared_ptr<InstanceObject>,
                                 std::shared_ptr<BoundMethod>, std::shared_ptr<RangeObject>,
                                 std::shared_ptr<IteratorObject>, std::shared_ptr<ListObject>,
                                 std::shared_ptr<DictionaryObject>, std::shared_ptr<DictionaryIteratorObject>>;
    Storage data;

    Value() = default;
    Value(std::nullptr_t) : data(std::monostate{}) {}
    Value(bool value) : data(value) {}
    Value(double value) : data(value) {}
    Value(std::string value) : data(std::move(value)) {}
    Value(const char* value) : data(std::string(value)) {}
    Value(std::shared_ptr<NativeFunction> value) : data(std::move(value)) {}
    Value(std::shared_ptr<FunctionObject> value) : data(std::move(value)) {}
    Value(std::shared_ptr<ClassObject> value) : data(std::move(value)) {}
    Value(std::shared_ptr<InstanceObject> value) : data(std::move(value)) {}
    Value(std::shared_ptr<BoundMethod> value) : data(std::move(value)) {}
    Value(std::shared_ptr<RangeObject> value) : data(std::move(value)) {}
    Value(std::shared_ptr<IteratorObject> value) : data(std::move(value)) {}
    Value(std::shared_ptr<ListObject> value) : data(std::move(value)) {}
    Value(std::shared_ptr<DictionaryObject> value) : data(std::move(value)) {}
    Value(std::shared_ptr<DictionaryIteratorObject> value) : data(std::move(value)) {}
};

struct ListObject { std::vector<Value> values; };

struct DictionaryObject {
    std::vector<std::pair<Value, Value>> entries;
    std::optional<Value> get(const Value& key) const;
    void set(Value key, Value value);
};

struct RangeObject {
    std::int64_t start = 0;
    std::int64_t step = 1;
    std::int64_t stop = 0;
};

struct IteratorObject {
    explicit IteratorObject(std::shared_ptr<RangeObject> source)
        : range(std::move(source)), current(range->start) {}

    std::shared_ptr<RangeObject> range;
    std::int64_t current = 0;
};

struct DictionaryIteratorObject {
    enum class Projection { Pair, Keys, Values };
    explicit DictionaryIteratorObject(std::shared_ptr<DictionaryObject> source,
                                      Projection projection = Projection::Pair)
        : dictionary(std::move(source)), mode(projection) {}
    std::shared_ptr<DictionaryObject> dictionary;
    std::vector<std::pair<Value, Value>> snapshot;
    Projection mode;
    std::size_t index = 0;
};

struct NativeFunction {
    using Callback = std::function<Value(VM&, const std::vector<Value>&, const SourceSpan&)>;
    std::string name;
    std::size_t arity = 0;
    Callback callback;
};

struct FunctionObject {
    std::shared_ptr<FunctionPrototype> prototype;
    std::weak_ptr<Environment> closure;
};

struct Environment {
    explicit Environment(std::shared_ptr<Environment> parent_scope = nullptr)
        : parent(std::move(parent_scope)) {}

    bool contains_local(const std::string& name) const;
    Value get(const std::string& name, const SourceSpan& span) const;
    void define(const std::string& name, Value value);
    void define_constant(const std::string& name, Value value);
    void make_constant(const std::string& name, const SourceSpan& span);
    void assign(const std::string& name, Value value, const SourceSpan& span);

    std::shared_ptr<Environment> parent;
    std::unordered_map<std::string, Value> values;
    std::unordered_set<std::string> constants;
};

struct ClassObject {
    std::string name;
    std::shared_ptr<ClassObject> superclass;
    std::unordered_map<std::string, std::shared_ptr<FunctionObject>> methods;

    std::shared_ptr<FunctionObject> find_method(const std::string& method_name) const;
};

struct InstanceObject {
    explicit InstanceObject(std::shared_ptr<ClassObject> klass_value) : klass(std::move(klass_value)) {}

    std::shared_ptr<ClassObject> klass;
    std::unordered_map<std::string, Value> fields;
};

struct BoundMethod {
    std::shared_ptr<FunctionObject> function;
    std::shared_ptr<InstanceObject> receiver;
};

std::string value_to_string(const Value& value);
bool value_is_truthy(const Value& value);
bool values_equal(const Value& left, const Value& right);

}  // namespace novi
