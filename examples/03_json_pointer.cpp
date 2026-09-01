#include <iostream>
#include <senko/senko.hpp>

using json = senko::json;
using namespace senko::literals;

int main() {
    std::cout << "=== SenkoJSON v2.0 - RFC 6901 JSON Pointer Demo ===\n\n";

    json doc = R"({
        "store": {
            "book": [
                {
                    "category": "reference",
                    "author": "Nigel Rees",
                    "title": "Sayings of the Century",
                    "price": 8.95
                },
                {
                    "category": "fiction",
                    "author": "Evelyn Waugh",
                    "title": "Sword of Honour",
                    "price": 12.99
                }
            ],
            "bicycle": {
                "color": "red",
                "price": 19.95
            }
        },
        "escaped~slash/key": "working"
    })"_json;

    // 1. Direct resolution with json_pointer literal
    std::cout << "Book 0 Author: " << doc["/store/book/0/author"_json_pointer].get<std::string>() << "\n";
    std::cout << "Book 1 Title:  " << doc["/store/book/1/title"_json_pointer].get<std::string>() << "\n";
    std::cout << "Bicycle Price: " << doc["/store/bicycle/price"_json_pointer].get<double>() << "\n";

    // 2. Escaped characters (~0 -> ~, ~1 -> /)
    std::cout << "Escaped Key Value: " << doc["/escaped~0slash~1key"_json_pointer].get<std::string>() << "\n\n";

    // 3. Modifying through pointer
    doc["/store/bicycle/price"_json_pointer] = 24.99;
    std::cout << "Updated Bicycle Price: " << doc["/store/bicycle/price"_json_pointer].get<double>() << "\n";

    return 0;
}
