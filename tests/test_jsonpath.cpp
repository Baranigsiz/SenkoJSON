#include "test_framework.hpp"
#include <senko/senko.hpp>

using json = senko::json;

TEST_CASE("JSONPath - Dot & Bracket Child Access") {
    json doc = json::parse(R"({
        "store": {
            "name": "Senko Bookshop",
            "city": "Tokyo"
        }
    })");

    auto name_res = doc.jsonpath("$.store.name");
    CHECK_EQ(name_res.size(), 1);
    CHECK_EQ(name_res[0].get<std::string>(), "Senko Bookshop");

    auto city_res = doc.jsonpath("$['store']['city']");
    CHECK_EQ(city_res.size(), 1);
    CHECK_EQ(city_res[0].get<std::string>(), "Tokyo");

    CHECK_EQ(doc.jsonpath_first("$.store.name").get<std::string>(), "Senko Bookshop");
}

TEST_CASE("JSONPath - Array Wildcards & Slices") {
    json doc = json::parse(R"({
        "books": [
            {"title": "Book A", "price": 10},
            {"title": "Book B", "price": 20},
            {"title": "Book C", "price": 30}
        ]
    })");

    // All titles via wildcard
    auto titles = doc.jsonpath("$.books[*].title");
    CHECK_EQ(titles.size(), 3);
    CHECK_EQ(titles[0].get<std::string>(), "Book A");
    CHECK_EQ(titles[1].get<std::string>(), "Book B");
    CHECK_EQ(titles[2].get<std::string>(), "Book C");

    // Index access
    auto first_book = doc.jsonpath("$.books[0].title");
    CHECK_EQ(first_book.size(), 1);
    CHECK_EQ(first_book[0].get<std::string>(), "Book A");

    // Negative index
    auto last_book = doc.jsonpath("$.books[-1].title");
    CHECK_EQ(last_book.size(), 1);
    CHECK_EQ(last_book[0].get<std::string>(), "Book C");
}

TEST_CASE("JSONPath - Recursive Descent (..)") {
    json doc = json::parse(R"({
        "level1": {
            "author": "Author 1",
            "level2": {
                "author": "Author 2",
                "level3": {
                    "author": "Author 3"
                }
            }
        }
    })");

    auto authors = doc.jsonpath("$..author");
    CHECK_EQ(authors.size(), 3);
    CHECK_EQ(authors[0].get<std::string>(), "Author 1");
    CHECK_EQ(authors[1].get<std::string>(), "Author 2");
    CHECK_EQ(authors[2].get<std::string>(), "Author 3");
}

TEST_CASE("JSONPath - Filter Expressions [?(@.field op val)]") {
    json doc = json::parse(R"({
        "products": [
            {"name": "Keyboard", "price": 45.0, "in_stock": true},
            {"name": "Mouse", "price": 15.0, "in_stock": true},
            {"name": "Monitor", "price": 150.0, "in_stock": false},
            {"name": "Pad", "price": 8.0, "in_stock": true}
        ]
    })");

    // Price < 20
    auto cheap = doc.jsonpath("$.products[?(@.price < 20)].name");
    CHECK_EQ(cheap.size(), 2);
    CHECK_EQ(cheap[0].get<std::string>(), "Mouse");
    CHECK_EQ(cheap[1].get<std::string>(), "Pad");

    // In stock == true
    auto in_stock = doc.jsonpath("$.products[?(@.in_stock == true)].name");
    CHECK_EQ(in_stock.size(), 3);

    // Exact string match
    auto keyboard = doc.jsonpath("$.products[?(@.name == 'Keyboard')].price");
    CHECK_EQ(keyboard.size(), 1);
    CHECK_EQ(keyboard[0].get<double>(), 45.0);
}
