#include "test_framework.hpp"
#include <senko/senko.hpp>

#include <vector>
#include <string>

using json = senko::json;

struct EventRecorder : public senko::default_sax_handler {
    std::vector<std::string> events;

    bool null() {
        events.push_back("null");
        return true;
    }
    bool boolean(bool val) {
        events.push_back(val ? "bool:true" : "bool:false");
        return true;
    }
    bool number_integer(int64_t val) {
        events.push_back("int:" + std::to_string(val));
        return true;
    }
    bool number_unsigned(uint64_t val) {
        events.push_back("uint:" + std::to_string(val));
        return true;
    }
    bool number_float(double val) {
        events.push_back("float:" + std::to_string(val));
        return true;
    }
    bool string(std::string_view val) {
        events.push_back("str:" + std::string(val));
        return true;
    }
    bool start_object(size_t = static_cast<size_t>(-1)) {
        events.push_back("start_obj");
        return true;
    }
    bool key(std::string_view k) {
        events.push_back("key:" + std::string(k));
        return true;
    }
    bool end_object() {
        events.push_back("end_obj");
        return true;
    }
    bool start_array(size_t = static_cast<size_t>(-1)) {
        events.push_back("start_arr");
        return true;
    }
    bool end_array() {
        events.push_back("end_arr");
        return true;
    }
};

TEST_CASE("SAX Parser - Full Event Recording") {
    std::string json_text = R"({
        "name": "Senko",
        "age": 500,
        "is_fox": true,
        "powers": ["cooking", "comforting"]
    })";

    EventRecorder recorder;
    bool ok = senko::sax_parse(json_text, recorder);

    CHECK(ok);
    CHECK(recorder.events.size() > 0);
    CHECK_EQ(recorder.events[0], "start_obj");
    CHECK_EQ(recorder.events[1], "key:name");
    CHECK_EQ(recorder.events[2], "str:Senko");
    CHECK_EQ(recorder.events[3], "key:age");
    CHECK_EQ(recorder.events[4], "int:500");
    CHECK_EQ(recorder.events[5], "key:is_fox");
    CHECK_EQ(recorder.events[6], "bool:true");
    CHECK_EQ(recorder.events[7], "key:powers");
    CHECK_EQ(recorder.events[8], "start_arr");
    CHECK_EQ(recorder.events[9], "str:cooking");
    CHECK_EQ(recorder.events[10], "str:comforting");
    CHECK_EQ(recorder.events[11], "end_arr");
    CHECK_EQ(recorder.events[12], "end_obj");
}

struct EarlyStopHandler : public senko::default_sax_handler {
    int string_count = 0;

    bool string(std::string_view) {
        string_count++;
        if (string_count >= 2) {
            return false; // Stop parsing early!
        }
        return true;
    }
};

TEST_CASE("SAX Parser - Early Stopping") {
    std::string json_text = R"(["first", "second", "third", "fourth", "fifth"])";
    EarlyStopHandler handler;
    bool ok = senko::sax_parse(json_text, handler);

    // Returned false because handler aborted on 2nd string
    CHECK(!ok);
    CHECK_EQ(handler.string_count, 2);
}
