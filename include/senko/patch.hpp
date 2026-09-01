#pragma once

#include "fwd.hpp"
#include "error.hpp"
#include "value.hpp"
#include "json_pointer.hpp"

#include <string>
#include <vector>
#include <utility>
#include <algorithm>

namespace senko {

/**
 * @brief Exception thrown when a JSON Patch (RFC 6902) operation fails.
 */
class patch_error : public exception {
public:
    explicit patch_error(std::string msg)
        : exception("[senko::patch_error] " + std::move(msg)) {}
};

/**
 * @brief Exception thrown when a JSON Patch 'test' assertion operation fails.
 */
class patch_test_failed : public patch_error {
public:
    explicit patch_test_failed(std::string msg)
        : patch_error("Test operation failed: " + std::move(msg)) {}
};

namespace detail {

inline void apply_patch_op(value& doc, const value& op_obj) {
    if (!op_obj.is_object()) {
        throw patch_error("Each patch operation must be a JSON object");
    }

    if (!op_obj.contains("op") || !op_obj.at("op").is_string()) {
        throw patch_error("Patch operation missing 'op' string property");
    }
    if (!op_obj.contains("path") || !op_obj.at("path").is_string()) {
        throw patch_error("Patch operation missing 'path' string property");
    }

    std::string op = op_obj.at("op").get<std::string>();
    std::string path_str = op_obj.at("path").get<std::string>();
    json_pointer ptr(path_str);

    if (op == "add") {
        if (!op_obj.contains("value")) {
            throw patch_error("Operation 'add' requires 'value' property");
        }
        const value& val = op_obj.at("value");

        if (ptr.empty()) {
            doc = val;
            return;
        }

        // Navigate to parent
        json_pointer parent_ptr = ptr;
        std::string last_token = parent_ptr.tokens().back();
        parent_ptr.pop_back();

        value& parent = parent_ptr.resolve(doc);
        if (parent.is_object()) {
            parent[last_token] = val;
        } else if (parent.is_array()) {
            auto& arr = parent.get_ref_array();
            if (last_token == "-") {
                arr.push_back(val);
            } else {
                size_t idx = 0;
                try {
                    idx = std::stoull(last_token);
                } catch (...) {
                    throw patch_error("Invalid array index in JSON Patch: '" + last_token + "'");
                }
                if (idx > arr.size()) {
                    throw out_of_range("Array index out of range for add operation: " + std::to_string(idx));
                }
                arr.insert(arr.begin() + idx, val);
            }
        } else {
            throw type_error("Cannot add element to primitive JSON value at path: " + parent_ptr.to_string());
        }

    } else if (op == "remove") {
        if (ptr.empty()) {
            throw patch_error("Cannot remove root JSON document");
        }

        json_pointer parent_ptr = ptr;
        std::string last_token = parent_ptr.tokens().back();
        parent_ptr.pop_back();

        value& parent = parent_ptr.resolve(doc);
        if (parent.is_object()) {
            if (!parent.contains(last_token)) {
                throw out_of_range("Key not found for remove operation: '" + last_token + "'");
            }
            parent.erase(last_token);
        } else if (parent.is_array()) {
            auto& arr = parent.get_ref_array();
            size_t idx = 0;
            try {
                idx = std::stoull(last_token);
            } catch (...) {
                throw patch_error("Invalid array index in JSON Patch: '" + last_token + "'");
            }
            if (idx >= arr.size()) {
                throw out_of_range("Array index out of range for remove operation: " + std::to_string(idx));
            }
            arr.erase(arr.begin() + idx);
        } else {
            throw type_error("Cannot remove element from primitive value at path: " + parent_ptr.to_string());
        }

    } else if (op == "replace") {
        if (!op_obj.contains("value")) {
            throw patch_error("Operation 'replace' requires 'value' property");
        }
        const value& val = op_obj.at("value");

        if (ptr.empty()) {
            doc = val;
            return;
        }

        // Must exist before replacing
        value& target = ptr.resolve(doc);
        target = val;

    } else if (op == "move") {
        if (!op_obj.contains("from") || !op_obj.at("from").is_string()) {
            throw patch_error("Operation 'move' requires 'from' string property");
        }
        std::string from_str = op_obj.at("from").get<std::string>();
        json_pointer from_ptr(from_str);

        if (from_ptr.empty()) {
            throw patch_error("Cannot move from root JSON document");
        }

        // Value to move
        value val = from_ptr.resolve(doc);

        // Remove from source
        value remove_op = value::object({{"op", "remove"}, {"path", from_str}});
        apply_patch_op(doc, remove_op);

        // Add to destination
        value add_op = value::object({{"op", "add"}, {"path", path_str}, {"value", std::move(val)}});
        apply_patch_op(doc, add_op);

    } else if (op == "copy") {
        if (!op_obj.contains("from") || !op_obj.at("from").is_string()) {
            throw patch_error("Operation 'copy' requires 'from' string property");
        }
        std::string from_str = op_obj.at("from").get<std::string>();
        json_pointer from_ptr(from_str);

        value val = from_ptr.resolve(doc);

        value add_op = value::object({{"op", "add"}, {"path", path_str}, {"value", std::move(val)}});
        apply_patch_op(doc, add_op);

    } else if (op == "test") {
        if (!op_obj.contains("value")) {
            throw patch_error("Operation 'test' requires 'value' property");
        }
        const value& expected = op_obj.at("value");
        const value& actual = ptr.resolve(doc);

        if (actual != expected) {
            throw patch_test_failed("Value at path '" + path_str + "' does not match expected value");
        }

    } else {
        throw patch_error("Unsupported JSON Patch operation: '" + op + "'");
    }
}

inline void generate_diff_recursive(const value& source, const value& target, const std::string& path, value::array_t& patches) {
    if (source == target) {
        return;
    }

    if (source.is_object() && target.is_object()) {
        const auto& src_obj = source.get_ref_object();
        const auto& tgt_obj = target.get_ref_object();

        // Check for removed keys
        for (const auto& [k, v] : src_obj) {
            if (!target.contains(k)) {
                std::string item_path = path + "/" + json_pointer::escape(k);
                patches.push_back(value::object({
                    {"op", "remove"},
                    {"path", item_path}
                }));
            }
        }

        // Check for added or modified keys
        for (const auto& [k, v] : tgt_obj) {
            std::string item_path = path + "/" + json_pointer::escape(k);
            if (!source.contains(k)) {
                patches.push_back(value::object({
                    {"op", "add"},
                    {"path", item_path},
                    {"value", v}
                }));
            } else {
                generate_diff_recursive(source.at(k), v, item_path, patches);
            }
        }
    } else if (source.is_array() && target.is_array()) {
        const auto& src_arr = source.get_ref_array();
        const auto& tgt_arr = target.get_ref_array();
        size_t min_len = (std::min)(src_arr.size(), tgt_arr.size());

        // Diffs in common indices
        for (size_t i = 0; i < min_len; ++i) {
            std::string item_path = path + "/" + std::to_string(i);
            generate_diff_recursive(src_arr[i], tgt_arr[i], item_path, patches);
        }

        // Extra elements in target (add)
        for (size_t i = min_len; i < tgt_arr.size(); ++i) {
            patches.push_back(value::object({
                {"op", "add"},
                {"path", path + "/-"},
                {"value", tgt_arr[i]}
            }));
        }

        // Extra elements in source (remove in reverse order)
        for (size_t i = src_arr.size(); i > min_len; --i) {
            patches.push_back(value::object({
                {"op", "remove"},
                {"path", path + "/" + std::to_string(i - 1)}
            }));
        }
    } else {
        // Types or primitives differ -> replace
        patches.push_back(value::object({
            {"op", "replace"},
            {"path", path.empty() ? "" : path},
            {"value", target}
        }));
    }
}

} // namespace detail

/**
 * @brief Applies an RFC 6902 JSON Patch document to the given JSON value in-place.
 */
inline void apply_patch(value& doc, const value& patch_doc) {
    if (!patch_doc.is_array()) {
        throw patch_error("JSON Patch document must be an array of operation objects");
    }
    for (size_t i = 0; i < patch_doc.size(); ++i) {
        detail::apply_patch_op(doc, patch_doc[i]);
    }
}

/**
 * @brief Computes the RFC 6902 JSON Patch that transforms `source` into `target`.
 */
inline value diff(const value& source, const value& target) {
    value::array_t patches;
    detail::generate_diff_recursive(source, target, "", patches);
    return value(std::move(patches));
}

// Inline member methods for value
inline value value::patch(const value& patch_doc) const {
    value copy = *this;
    apply_patch(copy, patch_doc);
    return copy;
}

inline void value::patch_in_place(const value& patch_doc) {
    apply_patch(*this, patch_doc);
}

inline value value::diff(const value& source, const value& target) {
    return senko::diff(source, target);
}

} // namespace senko
