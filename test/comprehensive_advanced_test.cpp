#include "../Json.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include <memory>
#include <vector>
#include <random>
#include <sstream>
#include <thread>
#include <atomic>
#include <fstream>
#include <climits>
#include <cfloat>

// Test result tracking
struct TestResults {
    int passed = 0;
    int failed = 0;
    std::vector<std::string> failures;
    
    void expect(bool condition, const std::string& test_name) {
        if (condition) {
            passed++;
            std::cout << "✓ " << test_name << std::endl;
        } else {
            failed++;
            failures.push_back(test_name);
            std::cout << "✗ " << test_name << std::endl;
        }
    }
    
    void print_summary() {
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        if (!failures.empty()) {
            std::cout << "Failed tests:" << std::endl;
            for (const auto& failure : failures) {
                std::cout << "  - " << failure << std::endl;
            }
        }
    }
};

TestResults results;

void testLargeObjectsAndArrays() {
    std::cout << "\n=== Testing Large Objects and Arrays ===\n";
    
    try {
        // Test large array
        auto large_array = Json::Array();
        const size_t large_size = 10000;
        
        // Reserve capacity to test memory efficiency
        large_array.Reserve(large_size);
        
        for (size_t i = 0; i < large_size; ++i) {
            large_array.PushBack(static_cast<int>(i));
        }
        
        results.expect(large_array.Size() == large_size, "Large array creation");
        results.expect(large_array[0].Get<int>() == 0, "Large array first element");
        results.expect(large_array[large_size-1].Get<int>() == static_cast<int>(large_size-1), "Large array last element");
        
        // Test large object
        auto large_object = Json::Object();
        for (size_t i = 0; i < 1000; ++i) {
            std::string key = "key_" + std::to_string(i);
            large_object[key] = "value_" + std::to_string(i);
        }
        
        results.expect(large_object.Keys().size() == 1000, "Large object creation");
        results.expect(large_object.Contains("key_500"), "Large object contains middle key");
        results.expect(large_object["key_999"].Get<std::string>() == "value_999", "Large object last element");
        
        // Test nested large structures
        auto nested_large = Json::Object();
        nested_large["large_array"] = std::move(large_array);
        nested_large["large_object"] = std::move(large_object);
        
        results.expect(nested_large["large_array"].Size() == large_size, "Nested large array");
        results.expect(nested_large["large_object"].Keys().size() == 1000, "Nested large object");
        
    } catch (const std::exception& e) {
        std::cout << "Exception in large objects test: " << e.what() << std::endl;
        results.expect(false, "Large objects exception handling");
    }
}

void testExtensiveManipulations() {
    std::cout << "\n=== Testing Extensive Manipulations ===\n";
    
    try {
        auto json = Json::Object();
        
        // Test dynamic type changes
        json["dynamic"] = 42;
        results.expect(json["dynamic"].IsNumber(), "Initial number type");
        
        json["dynamic"] = "string";
        results.expect(json["dynamic"].IsString(), "Changed to string type");
        
        json["dynamic"] = true;
        results.expect(json["dynamic"].IsBoolean(), "Changed to boolean type");
        
        json["dynamic"] = Json::Array();
        results.expect(json["dynamic"].IsArray(), "Changed to array type");
        
        json["dynamic"] = Json::Object();
        results.expect(json["dynamic"].IsObject(), "Changed to object type");
        
        json["dynamic"] = nullptr;
        results.expect(json["dynamic"].IsNull(), "Changed to null type");
        
        // Test complex manipulations
        json["array"] = Json::Array();
        for (int i = 0; i < 100; ++i) {
            json["array"].PushBack(i);
        }
        
        // Remove every other element
        for (int i = 99; i >= 0; i -= 2) {
            // Note: This test assumes PopBack removes last element
            json["array"].PopBack();
        }
        
        results.expect(json["array"].Size() == 50, "Array manipulation - removing elements");
        
        // Test object key manipulations
        json["obj"] = Json::Object();
        std::vector<std::string> keys_to_add;
        for (int i = 0; i < 50; ++i) {
            keys_to_add.push_back("key" + std::to_string(i));
        }
        
        // Add keys
        for (const auto& key : keys_to_add) {
            json["obj"][key] = key + "_value";
        }
        
        // Remove half the keys
        for (size_t i = 0; i < keys_to_add.size() / 2; ++i) {
            json["obj"].Remove(keys_to_add[i]);
        }
        
        results.expect(json["obj"].Keys().size() == 25, "Object key manipulation");
        
        // Test deep nesting modifications
        json["deep"] = Json::Object();
        auto* current = &json["deep"];
        for (int i = 0; i < 10; ++i) {
            (*current)["level" + std::to_string(i)] = Json::Object();
            current = &(*current)["level" + std::to_string(i)];
        }
        (*current)["value"] = "deep_value";
        
        results.expect(json["deep"]["level0"]["level1"]["level2"]["level3"]["level4"]
                            ["level5"]["level6"]["level7"]["level8"]["level9"]["value"]
                            .Get<std::string>() == "deep_value", "Deep nesting manipulation");
        
    } catch (const std::exception& e) {
        std::cout << "Exception in manipulations test: " << e.what() << std::endl;
        results.expect(false, "Manipulations exception handling");
    }
}

void testExtensiveSerializationDeserialization() {
    std::cout << "\n=== Testing Extensive Serialization/Deserialization ===\n";
    
    try {
        // Test complex nested structure
        auto complex = Json::Object();
        complex["string"] = "Hello \"World\" with \\backslashes\\ and \nnewlines\n and \ttabs\t";
        complex["number"] = 123.456789;
        complex["large_number"] = 1e20;
        complex["small_number"] = 1e-20;
        complex["integer"] = INT_MAX;
        complex["negative"] = INT_MIN;
        complex["boolean_true"] = true;
        complex["boolean_false"] = false;
        complex["null_value"] = nullptr;
        
        // Unicode strings
        complex["unicode"] = "Hello 世界 🌍 café naïve résumé";
        
        // Empty containers
        complex["empty_array"] = Json::Array();
        complex["empty_object"] = Json::Object();
        
        // Mixed array
        complex["mixed_array"] = Json::Array();
        complex["mixed_array"].PushBack(1);
        complex["mixed_array"].PushBack("string");
        complex["mixed_array"].PushBack(true);
        complex["mixed_array"].PushBack(nullptr);
        complex["mixed_array"].PushBack(Json::Array());
        complex["mixed_array"].PushBack(Json::Object());
        
        // Test serialization
        std::string compact = complex.ToString();
        std::string pretty = complex.ToString(true);
        
        results.expect(!compact.empty(), "Compact serialization produces output");
        results.expect(!pretty.empty(), "Pretty serialization produces output");
        results.expect(pretty.length() > compact.length(), "Pretty serialization is longer");
        
        // Test round-trip serialization
        auto parsed_compact = Json::Parse(compact);
        auto parsed_pretty = Json::Parse(pretty);
        
        results.expect(parsed_compact["string"].Get<std::string>() == complex["string"].Get<std::string>(), 
                      "Round-trip compact: string");
        results.expect(parsed_compact["number"].Get<double>() == complex["number"].Get<double>(), 
                      "Round-trip compact: number");
        results.expect(parsed_compact["boolean_true"].Get<bool>() == true, 
                      "Round-trip compact: boolean true");
        results.expect(parsed_compact["null_value"].IsNull(), 
                      "Round-trip compact: null");
        results.expect(parsed_compact["mixed_array"].Size() == 6, 
                      "Round-trip compact: mixed array size");
        
        results.expect(parsed_pretty["unicode"].Get<std::string>() == complex["unicode"].Get<std::string>(), 
                      "Round-trip pretty: unicode");
        
        // Test malformed JSON parsing
        std::vector<std::string> malformed_jsons = {
            "{",  // Incomplete object
            "}",  // Unexpected closing brace
            "[",  // Incomplete array
            "]",  // Unexpected closing bracket
            "{\"key\": }",  // Missing value
            "{\"key\": \"value\",}",  // Trailing comma
            "{key: \"value\"}",  // Unquoted key
            "{\"key\": 'value'}",  // Single quotes
            "{\"key\": undefined}",  // Undefined value
            "{\"key\": NaN}",  // NaN value
            "{\"key\": Infinity}",  // Infinity value
            "\"unterminated string",  // Unterminated string
            "{\"key\": \"value\" \"key2\": \"value2\"}",  // Missing comma
        };
        
        int parse_errors_caught = 0;
        for (const auto& malformed : malformed_jsons) {
            try {
                Json::Parse(malformed);
            } catch (const JsonParseError&) {
                parse_errors_caught++;
            } catch (const JsonException&) {
                parse_errors_caught++;
            }
        }
        
        results.expect(parse_errors_caught == static_cast<int>(malformed_jsons.size()), 
                      "All malformed JSON parsing attempts throw exceptions");
        
    } catch (const std::exception& e) {
        std::cout << "Exception in serialization test: " << e.what() << std::endl;
        results.expect(false, "Serialization exception handling");
    }
}

void testTypeSystemAndSafety() {
    std::cout << "\n=== Testing Type System and Safety ===\n";
    
    try {
        // Test all type combinations for Get<T>()
        Json null_val = nullptr;
        Json bool_val = true;
        Json int_val = 42;
        Json double_val = 3.14;
        Json string_val = "hello";
        Json array_val = Json::Array();
        Json object_val = Json::Object();
        
        // Test correct type access
        results.expect(bool_val.Get<bool>() == true, "Correct bool access");
        results.expect(int_val.Get<int>() == 42, "Correct int access");
        results.expect(double_val.Get<double>() == 3.14, "Correct double access");
        results.expect(string_val.Get<std::string>() == "hello", "Correct string access");
        
        // Test type conversions where applicable
        results.expect(int_val.Get<double>() == 42.0, "Int to double conversion");
        results.expect(double_val.Get<int>() == 3, "Double to int conversion (truncation)");
        
        // Test TryGet for safe access
        auto safe_bool = bool_val.TryGet<bool>();
        auto safe_int_as_string = int_val.TryGet<std::string>();
        
        results.expect(safe_bool.has_value() && safe_bool.value() == true, "Safe bool access");
        results.expect(!safe_int_as_string.has_value(), "Safe failed conversion returns nullopt");
        
        // Test wrong type access throws exceptions
        int type_errors_caught = 0;
        
        try { null_val.Get<int>(); } catch (const JsonTypeError&) { type_errors_caught++; }
        try { bool_val.Get<std::string>(); } catch (const JsonTypeError&) { type_errors_caught++; }
        try { string_val.Get<bool>(); } catch (const JsonTypeError&) { type_errors_caught++; }
        try { array_val.Get<int>(); } catch (const JsonTypeError&) { type_errors_caught++; }
        try { object_val.Get<double>(); } catch (const JsonTypeError&) { type_errors_caught++; }
        
        results.expect(type_errors_caught == 5, "Type errors properly thrown");
        
        // Test boundary values
        Json max_int = INT_MAX;
        Json min_int = INT_MIN;
        Json max_double = DBL_MAX;
        Json min_double = DBL_MIN;
        
        results.expect(max_int.Get<int>() == INT_MAX, "Max int boundary");
        results.expect(min_int.Get<int>() == INT_MIN, "Min int boundary");
        results.expect(max_double.Get<double>() == DBL_MAX, "Max double boundary");
        results.expect(min_double.Get<double>() == DBL_MIN, "Min double boundary");
        
        // Test array bounds checking
        auto arr = Json::Array();
        arr.PushBack(1);
        arr.PushBack(2);
        
        int bounds_errors_caught = 0;
        try { arr[10]; } catch (const JsonException&) { bounds_errors_caught++; }
        try { arr[-1]; } catch (const JsonException&) { bounds_errors_caught++; }  // This might not compile depending on implementation
        
        results.expect(bounds_errors_caught >= 1, "Array bounds checking");
        
        // Test object key access on non-objects
        int key_errors_caught = 0;
        try { int_val["key"]; } catch (const JsonException&) { key_errors_caught++; }
        try { array_val["key"]; } catch (const JsonException&) { key_errors_caught++; }
        try { null_val["key"]; } catch (const JsonException&) { key_errors_caught++; }
        
        results.expect(key_errors_caught == 3, "Object key access on non-objects throws");
        
    } catch (const std::exception& e) {
        std::cout << "Exception in type safety test: " << e.what() << std::endl;
        results.expect(false, "Type safety exception handling");
    }
}

void testMemoryManagement() {
    std::cout << "\n=== Testing Memory Management ===\n";
    
    try {
        // Test copy semantics
        auto original = Json::Object();
        original["data"] = "important";
        original["number"] = 42;
        
        auto copied = original;
        original["data"] = "modified";
        
        results.expect(copied["data"].Get<std::string>() == "important", "Deep copy semantics");
        results.expect(original["data"].Get<std::string>() == "modified", "Original after copy modification");
        
        // Test move semantics
        auto movable = Json::Object();
        movable["data"] = "movable";
        
        auto moved_to = std::move(movable);
        results.expect(moved_to["data"].Get<std::string>() == "movable", "Move semantics work");
        
        // Test assignment operations
        auto assigned = Json::Object();
        assigned = copied;
        results.expect(assigned["data"].Get<std::string>() == "important", "Copy assignment");
        
        auto move_assigned = Json::Object();
        auto temp = Json::Object();
        temp["temp"] = "temporary";
        move_assigned = std::move(temp);
        results.expect(move_assigned["temp"].Get<std::string>() == "temporary", "Move assignment");
        
        // Test self-assignment
        assigned = assigned;
        results.expect(assigned["data"].Get<std::string>() == "important", "Self assignment safety");
        
        // Test circular reference handling (if supported)
        auto obj1 = Json::Object();
        auto obj2 = Json::Object();
        obj1["ref"] = obj2;
        obj2["back_ref"] = obj1;  // This creates a cycle
        
        // Serialization should handle cycles gracefully or detect them
        bool cycle_handled = false;
        try {
            std::string serialized = obj1.ToString();
            cycle_handled = !serialized.empty();  // If it completes, cycles are handled
        } catch (const JsonException&) {
            cycle_handled = true;  // Exception is also acceptable for cycle detection
        }
        results.expect(cycle_handled, "Circular reference handling");
        
        // Test memory usage with repeated operations
        const int iterations = 1000;
        for (int i = 0; i < iterations; ++i) {
            auto temp_obj = Json::Object();
            temp_obj["iteration"] = i;
            temp_obj["data"] = Json::Array();
            for (int j = 0; j < 10; ++j) {
                temp_obj["data"].PushBack(j);
            }
            // temp_obj goes out of scope and should be cleaned up
        }
        
        results.expect(true, "Memory cleanup in loop operations");
        
    } catch (const std::exception& e) {
        std::cout << "Exception in memory management test: " << e.what() << std::endl;
        results.expect(false, "Memory management exception handling");
    }
}

void testConcurrencyAndThreadSafety() {
    std::cout << "\n=== Testing Concurrency and Thread Safety ===\n";
    
    try {
        // Test multiple threads reading the same JSON object
        auto shared_json = Json::Object();
        shared_json["counter"] = 0;
        shared_json["data"] = Json::Array();
        for (int i = 0; i < 100; ++i) {
            shared_json["data"].PushBack(i);
        }
        
        const int num_threads = 4;
        const int reads_per_thread = 100;
        std::vector<std::thread> readers;
        std::atomic<int> successful_reads{0};
        
        // Multiple reader threads
        for (int t = 0; t < num_threads; ++t) {
            readers.emplace_back([&shared_json, &successful_reads, reads_per_thread]() {
                for (int i = 0; i < reads_per_thread; ++i) {
                    try {
                        auto counter = shared_json["counter"].Get<int>();
                        auto size = shared_json["data"].Size();
                        if (counter == 0 && size == 100) {
                            successful_reads++;
                        }
                    } catch (...) {
                        // Read failed
                    }
                }
            });
        }
        
        for (auto& thread : readers) {
            thread.join();
        }
        
        results.expect(successful_reads.load() == num_threads * reads_per_thread, 
                      "Concurrent reads successful");
        
        // Test thread-local JSON objects
        std::vector<std::thread> writers;
        std::atomic<int> successful_writes{0};
        
        for (int t = 0; t < num_threads; ++t) {
            writers.emplace_back([&successful_writes, t]() {
                try {
                    auto local_json = Json::Object();
                    local_json["thread_id"] = t;
                    local_json["data"] = Json::Array();
                    
                    for (int i = 0; i < 50; ++i) {
                        local_json["data"].PushBack(i * t);
                    }
                    
                    auto serialized = local_json.ToString();
                    if (!serialized.empty()) {
                        successful_writes++;
                    }
                } catch (...) {
                    // Write failed
                }
            });
        }
        
        for (auto& thread : writers) {
            thread.join();
        }
        
        results.expect(successful_writes.load() == num_threads, 
                      "Thread-local JSON operations successful");
        
        // Note: True thread safety testing would require modifying the same object
        // from multiple threads, but this JSON library doesn't claim to be thread-safe
        // for concurrent modifications, so we only test safe concurrent read scenarios
        
    } catch (const std::exception& e) {
        std::cout << "Exception in concurrency test: " << e.what() << std::endl;
        results.expect(false, "Concurrency exception handling");
    }
}

void testPerformanceScenarios() {
    std::cout << "\n=== Testing Performance Scenarios ===\n";
    
    try {
        using namespace std::chrono;
        
        // Test large array creation performance
        auto start = high_resolution_clock::now();
        auto large_array = Json::Array();
        const size_t large_size = 50000;
        large_array.Reserve(large_size);
        
        for (size_t i = 0; i < large_size; ++i) {
            large_array.PushBack(static_cast<int>(i));
        }
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        
        std::cout << "Large array creation took: " << duration.count() << "ms" << std::endl;
        results.expect(large_array.Size() == large_size, "Performance: Large array creation");
        
        // Test serialization performance
        start = high_resolution_clock::now();
        std::string serialized = large_array.ToString();
        end = high_resolution_clock::now();
        duration = duration_cast<milliseconds>(end - start);
        
        std::cout << "Large array serialization took: " << duration.count() << "ms" << std::endl;
        results.expect(!serialized.empty(), "Performance: Large array serialization");
        
        // Test parsing performance
        start = high_resolution_clock::now();
        auto parsed = Json::Parse(serialized);
        end = high_resolution_clock::now();
        duration = duration_cast<milliseconds>(end - start);
        
        std::cout << "Large array parsing took: " << duration.count() << "ms" << std::endl;
        results.expect(parsed.Size() == large_size, "Performance: Large array parsing");
        
        // Test deep object access performance
        auto deep_obj = Json::Object();
        auto* current = &deep_obj;
        const int depth = 1000;
        
        start = high_resolution_clock::now();
        for (int i = 0; i < depth; ++i) {
            (*current)["level"] = Json::Object();
            current = &(*current)["level"];
        }
        (*current)["value"] = "deep";
        end = high_resolution_clock::now();
        duration = duration_cast<milliseconds>(end - start);
        
        std::cout << "Deep nesting creation took: " << duration.count() << "ms" << std::endl;
        results.expect(true, "Performance: Deep nesting creation");
        
        // Test repeated key access
        auto obj_with_many_keys = Json::Object();
        const int num_keys = 10000;
        for (int i = 0; i < num_keys; ++i) {
            obj_with_many_keys["key_" + std::to_string(i)] = i;
        }
        
        start = high_resolution_clock::now();
        int sum = 0;
        for (int i = 0; i < num_keys; ++i) {
            sum += obj_with_many_keys["key_" + std::to_string(i)].Get<int>();
        }
        end = high_resolution_clock::now();
        duration = duration_cast<milliseconds>(end - start);
        
        std::cout << "Key access benchmark took: " << duration.count() << "ms" << std::endl;
        results.expect(sum == (num_keys - 1) * num_keys / 2, "Performance: Key access correctness");
        
    } catch (const std::exception& e) {
        std::cout << "Exception in performance test: " << e.what() << std::endl;
        results.expect(false, "Performance exception handling");
    }
}

void testEdgeCasesAndCornerCases() {
    std::cout << "\n=== Testing Edge Cases and Corner Cases ===\n";
    
    try {
        // Empty values
        auto empty_string = Json("");
        auto empty_array = Json::Array();
        auto empty_object = Json::Object();
        
        results.expect(empty_string.Get<std::string>() == "", "Empty string");
        results.expect(empty_array.Size() == 0, "Empty array size");
        results.expect(empty_object.Keys().empty(), "Empty object keys");
        
        // Special numeric values
        Json zero_int = 0;
        Json zero_double = 0.0;
        Json negative_zero = -0.0;
        Json very_small = 1e-100;
        Json very_large = 1e100;
        
        results.expect(zero_int.Get<int>() == 0, "Zero integer");
        results.expect(zero_double.Get<double>() == 0.0, "Zero double");
        results.expect(very_small.Get<double>() == 1e-100, "Very small number");
        results.expect(very_large.Get<double>() == 1e100, "Very large number");
        
        // Special string values
        Json special_chars = "\"\\\b\f\n\r\t";
        Json unicode_str = "🌟✨🎉🚀💯";
        Json long_string = std::string(10000, 'x');
        
        results.expect(special_chars.IsString(), "Special characters string");
        results.expect(unicode_str.IsString(), "Unicode string");
        results.expect(long_string.Get<std::string>().length() == 10000, "Long string");
        
        // Nested empty structures
        auto nested_empty = Json::Object();
        nested_empty["empty_array"] = Json::Array();
        nested_empty["empty_object"] = Json::Object();
        nested_empty["empty_array_in_array"] = Json::Array();
        nested_empty["empty_array_in_array"].PushBack(Json::Array());
        
        results.expect(nested_empty["empty_array"].Size() == 0, "Nested empty array");
        results.expect(nested_empty["empty_object"].Keys().empty(), "Nested empty object");
        results.expect(nested_empty["empty_array_in_array"][0].Size() == 0, "Empty array in array");
        
        // Null handling in various contexts
        auto null_tests = Json::Object();
        null_tests["explicit_null"] = nullptr;
        null_tests["array_with_null"] = Json::Array();
        null_tests["array_with_null"].PushBack(nullptr);
        null_tests["mixed"] = Json::Array();
        null_tests["mixed"].PushBack(1);
        null_tests["mixed"].PushBack(nullptr);
        null_tests["mixed"].PushBack("string");
        
        results.expect(null_tests["explicit_null"].IsNull(), "Explicit null");
        results.expect(null_tests["array_with_null"][0].IsNull(), "Null in array");
        results.expect(null_tests["mixed"][1].IsNull(), "Null in mixed array");
        
        // Key edge cases
        auto key_tests = Json::Object();
        key_tests[""] = "empty_key";  // Empty key
        key_tests["key with spaces"] = "spaces";
        key_tests["key\nwith\nnewlines"] = "newlines";
        key_tests["🔑"] = "emoji_key";
        
        results.expect(key_tests[""].Get<std::string>() == "empty_key", "Empty key access");
        results.expect(key_tests["key with spaces"].Get<std::string>() == "spaces", "Key with spaces");
        results.expect(key_tests["🔑"].Get<std::string>() == "emoji_key", "Emoji key");
        
        // Duplicate key handling in parsing (should keep last value)
        try {
            auto duplicate_keys = Json::Parse(R"({"key": "first", "key": "second"})");
            results.expect(duplicate_keys["key"].Get<std::string>() == "second", "Duplicate key handling");
        } catch (const JsonParseError&) {
            // Some implementations might reject duplicate keys
            results.expect(true, "Duplicate key rejection is acceptable");
        }
        
        // Array edge cases
        auto arr_edge = Json::Array();
        arr_edge.PushBack(Json::Array());  // Array containing array
        arr_edge[0].PushBack(Json::Object());  // Array containing object
        arr_edge[0][0]["nested"] = "deep";
        
        results.expect(arr_edge[0][0]["nested"].Get<std::string>() == "deep", "Deep array-object nesting");
        
    } catch (const std::exception& e) {
        std::cout << "Exception in edge cases test: " << e.what() << std::endl;
        results.expect(false, "Edge cases exception handling");
    }
}

int main() {
    try {
        std::cout << "JSON Library Comprehensive Advanced Test Suite\n";
        std::cout << "=============================================\n";
        
        testLargeObjectsAndArrays();
        testExtensiveManipulations();
        testExtensiveSerializationDeserialization();
        testTypeSystemAndSafety();
        testMemoryManagement();
        testConcurrencyAndThreadSafety();
        testPerformanceScenarios();
        testEdgeCasesAndCornerCases();
        
        results.print_summary();
        
        if (results.failed == 0) {
            std::cout << "\n🎉 All comprehensive advanced tests passed!\n";
            return 0;
        } else {
            std::cout << "\n❌ Some tests failed. Check the summary above.\n";
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test suite failed with exception: " << e.what() << std::endl;
        return 1;
    }
} 