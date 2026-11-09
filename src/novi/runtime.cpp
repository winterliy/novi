#include "value.hpp"

#include <cmath>
#include <sstream>

namespace novi {

std::optional<Value> DictionaryObject::get(const Value& key) const {
    for (const auto& entry : entries) if (values_equal(entry.first, key)) return entry.second;
    return std::nullopt;
}

void DictionaryObject::set(Value key, Value value) {
    for (auto& entry : entries) {
        if (values_equal(entry.first, key)) {
            entry.second = std::move(value);
            return;
        }
    }
    entries.emplace_back(std::move(key), std::move(value));
}

bool Environment::contains_local(const std::string& name) const {
    return values.find(name) != values.end();
}

Value Environment::get(const std::string& name, const SourceSpan& span) const {
    const auto found = values.find(name);
    if (found != values.end()) return found->second;
    if (parent) return parent->get(name, span);
    throw RuntimeError("Undefined name '" + name + "'", span);
}

void Environment::define(const std::string& name, Value value) {
    values[name] = std::move(value);
    fixed_types.erase(name);
}

void Environment::define_constant(const std::string& name, Value value) {
    values[name] = std::move(value);
    fixed_types.erase(name);
    constants.insert(name);
}

void Environment::define_typed(const std::string& name, Value value, std::string type_name,
                               bool is_constant, const SourceSpan& span) {
    if (!value_matches_type(value, type_name))
        throw RuntimeError("Value does not match declared type '" + type_name + "'", span);
    values[name] = std::move(value);
    fixed_types[name] = std::move(type_name);
    if (is_constant) constants.insert(name);
    else constants.erase(name);
}

void Environment::make_constant(const std::string& name, const SourceSpan& span) {
    if (values.find(name) != values.end()) {
        constants.insert(name);
        return;
    }
    if (parent) {
        parent->make_constant(name, span);
        return;
    }
    throw RuntimeError("Undefined name '" + name + "'", span);
}

void Environment::assign(const std::string& name, Value value, const SourceSpan& span) {
    const auto found = values.find(name);
    if (found != values.end()) {
        if (constants.find(name) != constants.end())
            throw RuntimeError("Cannot reassign constant '" + name + "'", span);
        const auto type = fixed_types.find(name);
        if (type != fixed_types.end() && !value_matches_type(value, type->second))
            throw RuntimeError("Assigned value does not match fixed type '" + type->second + "'", span);
        found->second = std::move(value);
        return;
    }
    if (parent) {
        parent->assign(name, std::move(value), span);
        return;
    }
    throw RuntimeError("Undefined name '" + name + "'", span);
}

std::shared_ptr<FunctionObject> ClassObject::find_method(const std::string& method_name) const {
    const auto found = methods.find(method_name);
    if (found != methods.end()) return found->second;
    return superclass ? superclass->find_method(method_name) : nullptr;
}

std::shared_ptr<FunctionObject> ClassObject::find_static_method(const std::string& method_name) const {
    const auto found = static_methods.find(method_name);
    if (found != static_methods.end()) return found->second;
    return superclass ? superclass->find_static_method(method_name) : nullptr;
}

std::string value_to_string(const Value& value) {
    if (std::holds_alternative<std::monostate>(value.data)) return "null";
    if (const auto* boolean = std::get_if<bool>(&value.data)) return *boolean ? "true" : "false";
    if (const auto* number = std::get_if<double>(&value.data)) {
        std::ostringstream output;
        output.precision(15);
        output << *number;
        return output.str();
    }
    if (const auto* string = std::get_if<std::string>(&value.data)) return *string;
    if (const auto* native = std::get_if<std::shared_ptr<NativeFunction>>(&value.data)) return "<native " + (*native)->name + ">";
    if (const auto* function = std::get_if<std::shared_ptr<FunctionObject>>(&value.data)) return "<function " + (*function)->prototype->name + ">";
    if (const auto* klass = std::get_if<std::shared_ptr<ClassObject>>(&value.data)) return "<class " + (*klass)->name + ">";
    if (const auto* instance = std::get_if<std::shared_ptr<InstanceObject>>(&value.data)) return "<" + (*instance)->klass->name + " instance>";
    if (const auto* method = std::get_if<std::shared_ptr<BoundMethod>>(&value.data))
        return "<bound method " + (*method)->function->prototype->name + ">";
    if (const auto* range = std::get_if<std::shared_ptr<RangeObject>>(&value.data))
        return "range(" + std::to_string((*range)->start) + ", " + std::to_string((*range)->step) +
            ", " + std::to_string((*range)->stop) + ")";
    if (const auto* list = std::get_if<std::shared_ptr<ListObject>>(&value.data)) {
        std::ostringstream output;
        output << '[';
        for (std::size_t i = 0; i < (*list)->values.size(); ++i) {
            if (i) output << ", ";
            output << value_to_string((*list)->values[i]);
        }
        output << ']';
        return output.str();
    }
    if (const auto* dictionary = std::get_if<std::shared_ptr<DictionaryObject>>(&value.data)) {
        std::ostringstream output;
        output << "dictionary{";
        for (std::size_t i = 0; i < (*dictionary)->entries.size(); ++i) {
            if (i) output << ", ";
            output << value_to_string((*dictionary)->entries[i].first) << ": "
                   << value_to_string((*dictionary)->entries[i].second);
        }
        output << '}';
        return output.str();
    }
    if (const auto* set = std::get_if<std::shared_ptr<SetObject>>(&value.data)) {
        std::ostringstream output;
        output << "set{";
        for (std::size_t i = 0; i < (*set)->values.size(); ++i) {
            if (i) output << ", ";
            output << value_to_string((*set)->values[i]);
        }
        output << '}';
        return output.str();
    }
    if (const auto* queue = std::get_if<std::shared_ptr<QueueObject>>(&value.data)) {
        std::ostringstream output;
        output << "queue[";
        for (std::size_t i = 0; i < (*queue)->values.size(); ++i) {
            if (i) output << ", ";
            output << value_to_string((*queue)->values[i]);
        }
        output << ']';
        return output.str();
    }
    if (const auto* tuple = std::get_if<std::shared_ptr<TupleObject>>(&value.data)) {
        std::ostringstream output;
        output << "tuple(";
        for (std::size_t i = 0; i < (*tuple)->values.size(); ++i) {
            if (i) output << ", ";
            output << value_to_string((*tuple)->values[i]);
        }
        if ((*tuple)->values.size() == 1) output << ',';
        output << ')';
        return output.str();
    }
    if (const auto* iterator = std::get_if<std::shared_ptr<IteratorObject>>(&value.data)) {
        (void)iterator;
        return "<range iterator>";
    }
    if (std::holds_alternative<std::shared_ptr<DictionaryIteratorObject>>(value.data)) return "<dictionary iterator>";
    return "<collection iterator>";
}

bool value_is_truthy(const Value& value) {
    if (std::holds_alternative<std::monostate>(value.data)) return false;
    if (const auto* boolean = std::get_if<bool>(&value.data)) return *boolean;
    if (const auto* number = std::get_if<double>(&value.data)) return *number != 0.0;
    if (const auto* string = std::get_if<std::string>(&value.data)) return !string->empty();
    return true;
}

bool values_equal(const Value& left, const Value& right) {
    if (left.data.index() != right.data.index()) return false;
    if (std::holds_alternative<std::monostate>(left.data)) return true;
    if (const auto* value = std::get_if<bool>(&left.data)) return *value == std::get<bool>(right.data);
    if (const auto* value = std::get_if<double>(&left.data)) return *value == std::get<double>(right.data);
    if (const auto* value = std::get_if<std::string>(&left.data)) return *value == std::get<std::string>(right.data);
    if (const auto* value = std::get_if<std::shared_ptr<NativeFunction>>(&left.data)) return *value == std::get<std::shared_ptr<NativeFunction>>(right.data);
    if (const auto* value = std::get_if<std::shared_ptr<FunctionObject>>(&left.data)) return *value == std::get<std::shared_ptr<FunctionObject>>(right.data);
    if (const auto* value = std::get_if<std::shared_ptr<ClassObject>>(&left.data)) return *value == std::get<std::shared_ptr<ClassObject>>(right.data);
    if (const auto* value = std::get_if<std::shared_ptr<InstanceObject>>(&left.data)) return *value == std::get<std::shared_ptr<InstanceObject>>(right.data);
    if (const auto* value = std::get_if<std::shared_ptr<BoundMethod>>(&left.data)) return *value == std::get<std::shared_ptr<BoundMethod>>(right.data);
    if (const auto* value = std::get_if<std::shared_ptr<RangeObject>>(&left.data)) {
        const auto& other = std::get<std::shared_ptr<RangeObject>>(right.data);
        return (*value)->start == other->start && (*value)->step == other->step && (*value)->stop == other->stop;
    }
    if (const auto* value = std::get_if<std::shared_ptr<IteratorObject>>(&left.data))
        return *value == std::get<std::shared_ptr<IteratorObject>>(right.data);
    if (const auto* value = std::get_if<std::shared_ptr<ListObject>>(&left.data))
        return *value == std::get<std::shared_ptr<ListObject>>(right.data);
    if (const auto* value = std::get_if<std::shared_ptr<DictionaryObject>>(&left.data))
        return *value == std::get<std::shared_ptr<DictionaryObject>>(right.data);
    if (const auto* value = std::get_if<std::shared_ptr<DictionaryIteratorObject>>(&left.data))
        return *value == std::get<std::shared_ptr<DictionaryIteratorObject>>(right.data);
    if (const auto* value = std::get_if<std::shared_ptr<SequenceIteratorObject>>(&left.data))
        return *value == std::get<std::shared_ptr<SequenceIteratorObject>>(right.data);
    if (const auto* value = std::get_if<std::shared_ptr<SetObject>>(&left.data))
        return *value == std::get<std::shared_ptr<SetObject>>(right.data);
    if (const auto* value = std::get_if<std::shared_ptr<QueueObject>>(&left.data))
        return *value == std::get<std::shared_ptr<QueueObject>>(right.data);
    const auto& tuple = std::get<std::shared_ptr<TupleObject>>(left.data);
    const auto& other = std::get<std::shared_ptr<TupleObject>>(right.data);
    if (tuple == other) return true;
    if (!tuple || !other || tuple->values.size() != other->values.size()) return false;
    for (std::size_t i = 0; i < tuple->values.size(); ++i)
        if (!values_equal(tuple->values[i], other->values[i])) return false;
    return true;
}

bool value_matches_type(const Value& value, const std::string& type_name) {
    if (type_name == "boolean") return std::holds_alternative<bool>(value.data);
    if (type_name == "string") return std::holds_alternative<std::string>(value.data);
    if (type_name == "null" || type_name == "none") return std::holds_alternative<std::monostate>(value.data);
    if (type_name == "nan") {
        const auto* number = std::get_if<double>(&value.data);
        return number && std::isnan(*number);
    }
    if (type_name == "integer" || type_name == "natural" || type_name == "nonint" ||
        type_name == "realn" || type_name == "decimal" || type_name == "scalar") {
        const auto* number = std::get_if<double>(&value.data);
        if (!number || !std::isfinite(*number)) return false;
        if (type_name == "integer") return std::trunc(*number) == *number;
        if (type_name == "natural") return *number >= 0.0 && std::trunc(*number) == *number;
        if (type_name == "nonint") return std::trunc(*number) != *number;
        return true;
    }
    if (type_name == "list") return std::holds_alternative<std::shared_ptr<ListObject>>(value.data);
    if (type_name == "dictionary") return std::holds_alternative<std::shared_ptr<DictionaryObject>>(value.data);
    if (type_name == "set") return std::holds_alternative<std::shared_ptr<SetObject>>(value.data);
    if (type_name == "queue") return std::holds_alternative<std::shared_ptr<QueueObject>>(value.data);
    if (type_name == "tuple") return std::holds_alternative<std::shared_ptr<TupleObject>>(value.data);
    if (type_name == "range") return std::holds_alternative<std::shared_ptr<RangeObject>>(value.data);
    if (const auto* instance = std::get_if<std::shared_ptr<InstanceObject>>(&value.data))
        return *instance && (*instance)->klass->name == type_name;
    return false;
}

}  // namespace novi
