#include <iostream>
#include <senko/senko.hpp>

using json = senko::json;

int main() {
    std::cout << "=== SenkoJSON v2.1 - JSONPath (RFC 9535) Query Demo ===\n\n";

    json store = json::parse(R"({
        "store": {
            "book": [
                {"category": "reference", "author": "Nigel Rees", "title": "Sayings of the Century", "price": 8.95},
                {"category": "fiction", "author": "Evelyn Waugh", "title": "Sword of Honour", "price": 12.99},
                {"category": "fiction", "author": "Herman Melville", "title": "Moby Dick", "isbn": "0-553-21311-3", "price": 8.99},
                {"category": "fiction", "author": "J. R. R. Tolkien", "title": "The Lord of the Rings", "isbn": "0-395-19395-8", "price": 22.99}
            ],
            "bicycle": {
                "color": "red",
                "price": 19.95
            }
        }
    })");

    // 1. Query all book titles
    std::cout << "1. Query: '$.store.book[*].title'\n";
    auto titles = store.jsonpath("$.store.book[*].title");
    for (const auto& t : titles) {
        std::cout << "   - " << t.get<std::string>() << "\n";
    }
    std::cout << "\n";

    // 2. Recursive descent for all authors anywhere in the document
    std::cout << "2. Query: '$..author' (Recursive Descent)\n";
    auto authors = store.jsonpath("$..author");
    for (const auto& a : authors) {
        std::cout << "   - " << a.get<std::string>() << "\n";
    }
    std::cout << "\n";

    // 3. Filter expression: Books cheaper than $10.00
    std::cout << "3. Filter: '$.store.book[?(@.price < 10.0)].title'\n";
    auto cheap_books = store.jsonpath("$.store.book[?(@.price < 10.0)].title");
    for (const auto& b : cheap_books) {
        std::cout << "   - " << b.get<std::string>() << "\n";
    }
    std::cout << "\n";

    // 4. Filter expression: Fiction category only
    std::cout << "4. Filter: '$.store.book[?(@.category == 'fiction')].title'\n";
    auto fiction_books = store.jsonpath("$.store.book[?(@.category == 'fiction')].title");
    for (const auto& f : fiction_books) {
        std::cout << "   - " << f.get<std::string>() << "\n";
    }
    std::cout << "\n";

    return 0;
}
