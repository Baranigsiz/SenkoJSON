#include <iostream>
#include "corejson/JsonNode.h"

int main() {
    corejson::JsonNode root;
    root.type = corejson::JsonType::Object;
    root.object_values["name"] = corejson::JsonNode("CoreJSON");
    root.object_values["version"] = corejson::JsonNode(1.1);
    
    corejson::JsonNode features;
    features.type = corejson::JsonType::Array;
    features.array_values.push_back(corejson::JsonNode("Fast"));
    features.array_values.push_back(corejson::JsonNode("Small"));
    
    root.object_values["features"] = features;

    std::cout << "--- Pretty Print (4 spaces) ---\n";
    std::cout << root.dump(4) << "\n\n";

    std::cout << "--- Minified ---\n";
    std::cout << root.dump(-1) << "\n";

    return 0;
}
