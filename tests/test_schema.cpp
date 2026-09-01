#include "test_framework.hpp"
#include <senko/senko.hpp>

using json = senko::json;
using namespace senko::literals;

TEST_CASE("JSON Schema - Type Validation") {
    json schema_doc = R"({
        "type": "object",
        "properties": {
            "name": {"type": "string"},
            "age": {"type": "integer"},
            "rating": {"type": "number"},
            "active": {"type": "boolean"},
            "tags": {"type": "array"}
        },
        "required": ["name", "age"]
    })"_json;

    senko::schema s(schema_doc);

    // Valid instance
    json valid_inst = R"({
        "name": "Baran",
        "age": 25,
        "rating": 9.8,
        "active": true,
        "tags": ["cpp", "json"]
    })"_json;
    CHECK(s.validate(valid_inst));
    CHECK(valid_inst.validate(schema_doc));

    // Invalid instance (wrong type for age)
    json invalid_type = R"({
        "name": "Baran",
        "age": "twenty-five"
    })"_json;
    std::string err;
    CHECK(!invalid_type.validate(schema_doc, &err));
    CHECK(!err.empty());

    // Missing required field
    json missing_req = R"({
        "name": "Baran"
    })"_json;
    CHECK(!missing_req.validate(schema_doc));
}

TEST_CASE("JSON Schema - Numeric & String Constraints") {
    json schema_doc = R"({
        "type": "object",
        "properties": {
            "score": {
                "type": "number",
                "minimum": 0,
                "maximum": 100
            },
            "username": {
                "type": "string",
                "minLength": 3,
                "maxLength": 12,
                "pattern": "^[a-z_]+$"
            }
        }
    })"_json;

    senko::schema s(schema_doc);

    // Valid
    json valid_user = R"({"score": 95.5, "username": "senko_fox"})"_json;
    CHECK(s.validate(valid_user));

    // Score too high (> 100)
    json invalid_score = R"({"score": 150, "username": "senko_fox"})"_json;
    CHECK(!s.validate(invalid_score));

    // Username too short (< 3)
    json short_name = R"({"score": 50, "username": "ab"})"_json;
    CHECK(!s.validate(short_name));

    // Pattern mismatch (contains uppercase / numbers)
    json bad_pattern = R"({"score": 50, "username": "Senko123"})"_json;
    CHECK(!s.validate(bad_pattern));
}

TEST_CASE("JSON Schema - Array Constraints (minItems, uniqueItems, items)") {
    json schema_doc = R"({
        "type": "array",
        "minItems": 2,
        "maxItems": 4,
        "uniqueItems": true,
        "items": {
            "type": "integer"
        }
    })"_json;

    senko::schema s(schema_doc);

    // Valid
    json valid_arr = R"([10, 20, 30])"_json;
    CHECK(s.validate(valid_arr));

    // Non-unique items
    json dup_arr = R"([10, 20, 10])"_json;
    CHECK(!s.validate(dup_arr));

    // Too few items (< 2)
    json few_arr = R"([10])"_json;
    CHECK(!s.validate(few_arr));

    // Wrong item type
    json wrong_item = R"([10, "string", 30])"_json;
    CHECK(!s.validate(wrong_item));
}

TEST_CASE("JSON Schema - Combinators (allOf, anyOf, oneOf, not, enum)") {
    // 1. Enum validation
    json enum_schema = R"({
        "type": "string",
        "enum": ["red", "green", "blue"]
    })"_json;
    senko::schema s_enum(enum_schema);
    CHECK(s_enum.validate(json("red")));
    CHECK(!s_enum.validate(json("yellow")));

    // 2. oneOf validation (must match exactly one)
    json one_of_schema = R"({
        "oneOf": [
            {"type": "number", "maximum": 10},
            {"type": "number", "minimum": 20}
        ]
    })"_json;
    senko::schema s_one(one_of_schema);
    CHECK(s_one.validate(5_json));   // matches first only -> valid
    CHECK(s_one.validate(25_json));  // matches second only -> valid
    CHECK(!s_one.validate(15_json)); // matches neither -> invalid

    // 3. not validation
    json not_schema = R"({
        "not": {
            "type": "string"
        }
    })"_json;
    senko::schema s_not(not_schema);
    CHECK(s_not.validate(42_json));            // number is not string -> valid
    CHECK(!s_not.validate(json("hello")));    // string -> invalid
}
