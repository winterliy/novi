#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace novi {

struct SourceLocation {
    std::string file;
    std::size_t offset = 0;
    std::size_t line = 1;
    std::size_t column = 1;
};

struct SourceSpan {
    SourceLocation start;
    SourceLocation end;
};

class SourceFile {
public:
    SourceFile(std::string name, std::string text, std::size_t first_line = 1)
        : name_(std::move(name)), text_(std::move(text)), first_line_(first_line) {
        line_starts_.push_back(0);
        for (std::size_t i = 0; i < text_.size(); ++i) {
            if (text_[i] == '\n') line_starts_.push_back(i + 1);
        }
    }

    const std::string& name() const { return name_; }
    const std::string& text() const { return text_; }

    SourceLocation location(std::size_t offset) const {
        offset = offset > text_.size() ? text_.size() : offset;
        const auto next_line = std::upper_bound(line_starts_.begin(), line_starts_.end(), offset);
        const auto line_index = static_cast<std::size_t>(next_line - line_starts_.begin() - 1);
        return {name_, offset, first_line_ + line_index, offset - line_starts_[line_index] + 1};
    }

    SourceSpan span(std::size_t begin, std::size_t end) const {
        return {location(begin), location(end)};
    }

private:
    std::string name_;
    std::string text_;
    std::size_t first_line_;
    std::vector<std::size_t> line_starts_;
};

}  // namespace novi
