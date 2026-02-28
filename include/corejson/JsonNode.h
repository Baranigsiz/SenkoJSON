#pragma once
#include <string>
#include <vector>
#include <map>

namespace corejson {

enum class JsonType {
    String, Number, Boolean, Null, Object, Array
};

class JsonNode {
public:
    JsonType type;
    
    std::string string_value;
    double number_value;
    bool bool_value;
    std::map<std::string, JsonNode> object_values; 
    std::vector<JsonNode> array_values;            

    JsonNode();
    JsonNode(const std::string& value);
    JsonNode(const char* value); // ADDED: To prevent string literals converting to bool
    JsonNode(double value);
    JsonNode(bool value);
    
    void print(int indent = 0) const;

    // NEW: Function to convert the C++ tree back into a formatted JSON string
    std::string dump(int indent = 2, int currentIndent = 0) const;
};

} // namespace corejson