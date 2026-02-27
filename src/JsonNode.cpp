#include "corejson/JsonNode.h"
#include <iostream>
#include <sstream>

namespace corejson {

JsonNode::JsonNode() : type(JsonType::Null), number_value(0), bool_value(false) {}
JsonNode::JsonNode(const std::string& value) : type(JsonType::String), string_value(value), number_value(0), bool_value(false) {}
JsonNode::JsonNode(double value) : type(JsonType::Number), number_value(value), bool_value(false) {}
JsonNode::JsonNode(bool value) : type(JsonType::Boolean), bool_value(value), number_value(0) {}

void JsonNode::print(int indent) const {
    std::cout << dump(2, indent); // Artık dump fonksiyonumuzu print için de kullanabiliriz
}

// NEW: The Serializer logic
std::string JsonNode::dump(int indent, int currentIndent) const {
    std::string space(currentIndent, ' ');
    std::string nextSpace(currentIndent + indent, ' ');
    std::string result = "";

    if (type == JsonType::String) return "\"" + string_value + "\"";
    if (type == JsonType::Boolean) return bool_value ? "true" : "false";
    if (type == JsonType::Null) return "null";
    if (type == JsonType::Number) {
        std::ostringstream oss;
        oss << number_value;
        return oss.str();
    }
    
    if (type == JsonType::Array) {
        if (array_values.empty()) return "[]";
        result += "[\n";
        for (size_t i = 0; i < array_values.size(); ++i) {
            result += nextSpace + array_values[i].dump(indent, currentIndent + indent);
            if (i < array_values.size() - 1) result += ",";
            result += "\n";
        }
        result += space + "]";
        return result;
    }
    
    if (type == JsonType::Object) {
        if (object_values.empty()) return "{}";
        result += "{\n";
        size_t count = 0;
        for (const auto& pair : object_values) {
            result += nextSpace + "\"" + pair.first + "\": " + pair.second.dump(indent, currentIndent + indent);
            if (count < object_values.size() - 1) result += ",";
            result += "\n";
            count++;
        }
        result += space + "}";
        return result;
    }
    
    return "";
}

} // namespace corejson