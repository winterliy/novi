#include "value.hpp"

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
}

void Environment::define_constant(const std::string& name, Value value) {
    values[name] = std::move(value);
    constants.insert(name);
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
    if (const auto* iterator = std::get_if<std::shared_ptr<IteratorObject>>(&value.data)) {
        (void)iterator;
        return "<range iterator>";
    }
    return "<dictionary iterator>";
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
    return std::get<std::shared_ptr<DictionaryIteratorObject>>(left.data) ==
        std::get<std::shared_ptr<DictionaryIteratorObject>>(right.data);
}

}  // namespace novi
