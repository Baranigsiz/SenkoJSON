#pragma once

#include "fwd.hpp"
#include "value.hpp"
#include "error.hpp"

#include <string>
#include <string_view>
#include <istream>
#include <fstream>
#include <memory>
#include <utility>

namespace senko {

/**
 * @brief High-performance, streaming reader for JSON Lines / NDJSON (Newline Delimited JSON).
 * Memory efficient: processes gigabyte-scale datasets line-by-line with minimal RAM overhead.
 */
class jsonl_reader {
public:
    class iterator {
    public:
        using value_type = value;
        using difference_type = std::ptrdiff_t;
        using pointer = const value*;
        using reference = const value&;
        using iterator_category = std::input_iterator_tag;

        iterator() : m_reader(nullptr), m_line_num(0), m_done(true) {}

        explicit iterator(jsonl_reader* reader)
            : m_reader(reader), m_line_num(0), m_done(false) {
            advance();
        }

        reference operator*() const noexcept { return m_current; }
        pointer operator->() const noexcept { return &m_current; }

        iterator& operator++() {
            advance();
            return *this;
        }

        bool operator==(const iterator& other) const noexcept {
            if (m_done && other.m_done) return true;
            return m_done == other.m_done && m_reader == other.m_reader && m_line_num == other.m_line_num;
        }

        bool operator!=(const iterator& other) const noexcept {
            return !(*this == other);
        }

        size_t line_number() const noexcept { return m_line_num; }

    private:
        jsonl_reader* m_reader;
        size_t m_line_num;
        bool m_done;
        value m_current;

        void advance() {
            if (!m_reader) {
                m_done = true;
                return;
            }
            std::string line;
            while (m_reader->read_line(line)) {
                m_line_num++;
                // Skip empty lines or whitespace-only lines
                size_t start = 0;
                while (start < line.size() && (line[start] == ' ' || line[start] == '\t' || line[start] == '\r')) start++;
                if (start >= line.size()) continue;

                m_current = value::parse(std::string_view(line.data() + start, line.size() - start));
                return;
            }
            m_done = true;
        }
    };

    explicit jsonl_reader(std::string_view text)
        : m_type(source_type::memory), m_text(text), m_pos(0) {}

    explicit jsonl_reader(std::istream& is)
        : m_type(source_type::stream), m_stream(&is) {}

    static jsonl_reader from_file(const std::string& filepath) {
        return jsonl_reader(filepath, file_tag{});
    }

    iterator begin() { return iterator(this); }
    iterator end() { return iterator(); }

private:
    struct file_tag {};
    jsonl_reader(const std::string& filepath, file_tag)
        : m_type(source_type::file), m_file(std::make_unique<std::ifstream>(filepath, std::ios::in | std::ios::binary)) {
        if (!m_file->is_open()) {
            throw parse_error("Failed to open JSONL file: " + filepath);
        }
        m_stream = m_file.get();
    }

    enum class source_type { memory, stream, file };
    source_type m_type;
    std::string_view m_text;
    size_t m_pos = 0;
    std::unique_ptr<std::ifstream> m_file;
    std::istream* m_stream = nullptr;

    bool read_line(std::string& out) {
        out.clear();
        if (m_type == source_type::memory) {
            if (m_pos >= m_text.size()) return false;
            size_t next_nl = m_text.find('\n', m_pos);
            if (next_nl == std::string_view::npos) {
                out.assign(m_text.substr(m_pos));
                m_pos = m_text.size();
            } else {
                out.assign(m_text.substr(m_pos, next_nl - m_pos));
                m_pos = next_nl + 1;
            }
            return true;
        } else {
            if (!m_stream || !*m_stream) return false;
            if (std::getline(*m_stream, out)) {
                return true;
            }
            return false;
        }
    }

    friend class iterator;
};

namespace jsonl {
inline jsonl_reader from_file(const std::string& filepath) {
    return jsonl_reader::from_file(filepath);
}
inline jsonl_reader parse(std::string_view text) {
    return jsonl_reader(text);
}
inline jsonl_reader parse(std::istream& is) {
    return jsonl_reader(is);
}
} // namespace jsonl

} // namespace senko
