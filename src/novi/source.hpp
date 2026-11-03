#pragma once

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
        std::size_t line_index = 0;
        for (std::size_t i = 1; i < line_starts_.size(); ++i) {
            if (line_starts_[i] > offset) break;
            line_index = i;
        }
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
