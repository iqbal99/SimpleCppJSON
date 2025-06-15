#include "../Json.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <functional>

struct ErrorTest {
    std::string name;
    std::function<void()> test_func;
    bool should_throw;
    std::string expected_exception_type;
};

class ErrorTester {
private:
    std::vector<ErrorTest> tests;
    int passed = 0;
    int failed = 0;
    
public:
    void add_test(const std::string& name, std::function<void()> func, bool should_throw = true, const std::string& exception_type = "JsonException") {
        tests.push_back({name, func, should_throw, exception_type});
    }
    
    void run_all_tests() {
        std::cout << "Running " << tests.size() << " error handling tests...\n\n";
        
        for (const auto& test : tests) {
            run_single_test(test);
        }
        
        print_summary();
    }
    
private:
    void run_single_test(const ErrorTest& test) {
        std::cout << "Testing: " << test.name << " ... ";
        
        bool exception_thrown = false;
        std::string exception_type;
        
        try {
            test.test_func();
        } catch (const JsonParseError& e) {
            exception_thrown = true;
            exception_type = "JsonParseError";
        } catch (const JsonTypeError& e) {
            exception_thrown = true;
            exception_type = "JsonTypeError";
        } catch (const JsonException& e) {
            exception_thrown = true;
            exception_type = "JsonException";
        } catch (const std::exception& e) {
            exception_thrown = true;
            exception_type = "std::exception";
        }
        
        bool test_passed = (exception_thrown == test.should_throw);
        if (test_passed && test.should_throw) {
            // Additional check for exception type if needed
            if (!test.expected_exception_type.empty()) {
                test_passed = (exception_type == test.expected_exception_type || 
                              exception_type.find(test.expected_exception_type) != std::string::npos);
            }
        }
        
        if (test_passed) {
            std::cout << "✓ PASS";
            if (exception_thrown) {
                std::cout << " (threw " << exception_type << ")";
            }
            std::cout << std::endl;
            passed++;
        } else {
            std::cout << "✗ FAIL";
            if (test.should_throw && !exception_thrown) {
                std::cout << " (expected exception but none thrown)";
            } else if (!test.should_throw && exception_thrown) {
                std::cout << " (unexpected exception: " << exception_type << ")";
            }
            std::cout << std::endl;
            failed++;
        }
    }
    
    void print_summary() {
        std::cout << "\n=== Error Handling Test Summary ===\n";
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        std::cout << "Total:  " << (passed + failed) << std::endl;
        
        if (failed == 0) {
            std::cout << "🎉 All error handling tests passed!\n";
        } else {
            std::cout << "❌ Some error handling tests failed.\n";
        }
    }
};

void test_type_errors() {
    std::cout << "\n=== Type Error Tests ===\n";
    
    ErrorTester tester;
    
    // Test getting wrong types
    tester.add_test("Get int from string", []() {
        Json str = "hello";
        str.Get<int>();
    }, true, "JsonTypeError");
    
    tester.add_test("Get string from number", []() {
        Json num = 42;
        num.Get<std::string>();
    }, true, "JsonTypeError");
    
    tester.add_test("Get bool from array", []() {
        Json arr = Json::Array();
        arr.Get<bool>();
    }, true, "JsonTypeError");
    
    tester.add_test("Get double from object", []() {
        Json obj = Json::Object();
        obj.Get<double>();
    }, true, "JsonTypeError");
    
    tester.add_test("Get int from null", []() {
        Json null_val = nullptr;
        null_val.Get<int>();
    }, true, "JsonTypeError");
    
    // Test array operations on non-arrays
    // Note: These tests expect exceptions but the library might handle them gracefully
    tester.add_test("PushBack on number", []() {
        Json num = 42;
        num.PushBack(1);
    }, true, "JsonException");
    
    // Note: This library might auto-convert to array on index access
    tester.add_test("Array index on string", []() {
        Json str = "hello";
        auto& item = str[0];
        (void)item;
    }, true, "JsonException");
    
    // Size on boolean - RESTORED TO ORIGINAL EXPECTATION
    tester.add_test("Size on boolean - should throw", []() {
        Json bool_val = true;
        std::cout << "Calling Size() on boolean..." << std::endl;
        size_t size = bool_val.Size();
        std::cout << "Size() returned: " << size << " (expected exception but got value)" << std::endl;
    }, true, "JsonException");  // Restored to expect exception
    
    // Test object operations on non-objects
    // Note: This library might auto-convert to object on key access
    tester.add_test("Object key access on array", []() {
        Json arr = Json::Array();
        auto& item = arr["key"];
        (void)item;
    }, true, "JsonException");
    
    // Contains on number - RESTORED TO ORIGINAL EXPECTATION
    tester.add_test("Contains on number - should throw", []() {
        Json num = 42;
        std::cout << "Calling Contains() on number..." << std::endl;
        bool result = num.Contains("key");
        std::cout << "Contains() returned: " << result << " (expected exception but got value)" << std::endl;
    }, true, "JsonException");  // Restored to expect exception
    
    tester.add_test("Remove on string", []() {
        Json str = "hello";
        str.Remove("key");
    }, true, "JsonException");
    
    // Keys on boolean - RESTORED TO ORIGINAL EXPECTATION 
    tester.add_test("Keys on boolean - should throw", []() {
        Json bool_val = false;
        std::cout << "Calling Keys() on boolean..." << std::endl;
        auto keys = bool_val.Keys();
        std::cout << "Keys() returned vector with " << keys.size() << " elements (expected exception but got value)" << std::endl;
    }, true, "JsonException");  // Restored to expect exception
    
    tester.run_all_tests();
}

void test_boundary_conditions() {
    std::cout << "\n=== Boundary Condition Tests ===\n";
    
    ErrorTester tester;
    
    // Array bounds testing
    tester.add_test("Array access out of bounds - positive", []() {
        Json arr = Json::Array();
        arr.PushBack(1);
        arr.PushBack(2);
        auto& item = arr[10];  // Out of bounds
        (void)item;
    }, true, "JsonException");
    
    tester.add_test("Array access out of bounds - large index", []() {
        Json arr = Json::Array();
        auto& item = arr[SIZE_MAX];  // Very large index
        (void)item;
    }, true, "JsonException");
    
    tester.add_test("PopBack on empty array", []() {
        Json arr = Json::Array();
        arr.PopBack();  // Should handle gracefully or throw
    }, true, "JsonException");
    
    // Test very large numbers
    tester.add_test("Very large integer", []() {
        Json large_int = std::numeric_limits<int64_t>::max();
        large_int.Get<int64_t>();  // Should work
    }, false);
    
    tester.add_test("Very small integer", []() {
        Json small_int = std::numeric_limits<int64_t>::min();
        small_int.Get<int64_t>();  // Should work
    }, false);
    
    // Test extreme string lengths (this might not throw but could cause performance issues)
    tester.add_test("Very long string creation", []() {
        std::string very_long(100000, 'x');
        Json long_str = very_long;
        std::string retrieved = long_str.Get<std::string>();
        assert(retrieved.length() == 100000);
    }, false);
    
    tester.run_all_tests();
}

void test_parsing_errors() {
    std::cout << "\n=== Parsing Error Tests ===\n";
    
    ErrorTester tester;
    
    // Malformed JSON strings
    std::vector<std::pair<std::string, std::string>> malformed_jsons = {
        {"{", "Incomplete object"},
        {"}", "Unexpected closing brace"},
        {"[", "Incomplete array"},
        {"]", "Unexpected closing bracket"},
        {"{\"key\": }", "Missing value"},
        {"{\"key\": \"value\",}", "Trailing comma"},
        {"{key: \"value\"}", "Unquoted key"},
        {"{\"key\": 'value'}", "Single quotes"},
        {"{\"key\": undefined}", "Undefined value"},
        {"{\"key\": NaN}", "NaN value"},
        {"{\"key\": Infinity}", "Infinity value"},
        {"\"unterminated string", "Unterminated string"},
        {"{\"key\": \"value\" \"key2\": \"value2\"}", "Missing comma between pairs"},
        {"[1, 2, 3,]", "Trailing comma in array"},
        {"{\"key\": [1, 2, 3,]}", "Trailing comma in nested array"},
        {"null null", "Multiple values"},
        {"123abc", "Invalid number format"},
        {"{\"key\": 123.}", "Invalid decimal"},
        {"{\"key\": .123}", "Invalid decimal start"},
        {"{\"key\": 123.456.789}", "Multiple decimal points"},
        {"{\"key\": \"value\\\"}", "Invalid escape sequence"},
        {"{\"key\": \"\\u123\"}", "Invalid unicode escape"},
        {"{\"key\": \"\\uXYZ1\"}", "Invalid unicode characters"},
        {"", "Empty string"},
        {"   ", "Whitespace only"},
        {"{\"key1\": \"value1\" \"key2\": \"value2\"}", "Missing comma"},
        {"[1 2 3]", "Missing commas in array"},
        {"{\"key\": [1 2 3]}", "Missing commas in nested array"},
        {"{{\"key\": \"value\"}}", "Extra opening brace"},
        {"{\"key\": \"value\"}}}", "Extra closing braces"},
        // Note: These should actually parse correctly
        // {"[[1, 2, 3]]", "Double nested array - should work"},
        // {"{\"\\n\": \"value\"}", "Newline in key - should work"},
    };
    
    for (const auto& [json_str, description] : malformed_jsons) {
        tester.add_test("Parse: " + description, [json_str]() {
            Json::Parse(json_str);
        }, true, "JsonParseError");
    }
    
    // Test some edge cases that should NOT throw
    tester.add_test("Parse empty object", []() {
        auto result = Json::Parse("{}");
        assert(result.IsObject());
    }, false);
    
    tester.add_test("Parse empty array", []() {
        auto result = Json::Parse("[]");
        assert(result.IsArray());
    }, false);
    
    tester.add_test("Parse null", []() {
        auto result = Json::Parse("null");
        assert(result.IsNull());
    }, false);
    
    tester.add_test("Parse boolean true", []() {
        auto result = Json::Parse("true");
        assert(result.IsBoolean() && result.Get<bool>() == true);
    }, false);
    
    tester.add_test("Parse number zero", []() {
        auto result = Json::Parse("0");
        assert(result.IsNumber() && result.Get<int>() == 0);
    }, false);
    
    tester.add_test("Parse negative number", []() {
        auto result = Json::Parse("-123");
        assert(result.IsNumber() && result.Get<int>() == -123);
    }, false);
    
    // Test cases that should parse correctly  
    tester.add_test("Parse double nested array", []() {
        auto result = Json::Parse("[[1, 2, 3]]");
        assert(result.IsArray());
        assert(result.Size() == 1);
        assert(result[0].IsArray());
        assert(result[0].Size() == 3);
    }, false);
    
    tester.add_test("Parse escaped newline in key", []() {
        auto result = Json::Parse("{\"\\n\": \"value\"}");
        assert(result.IsObject());
        assert(result.Contains("\n"));
    }, false);
    
    tester.run_all_tests();
}

void test_serialization_errors() {
    std::cout << "\n=== Serialization Error Tests ===\n";
    
    ErrorTester tester;
    
    // Most serialization should not throw exceptions, but let's test edge cases
    tester.add_test("Serialize deeply nested structure", []() {
        auto deep = Json::Object();
        auto* current = &deep;
        for (int i = 0; i < 1000; ++i) {
            (*current)["level"] = Json::Object();
            current = &(*current)["level"];
        }
        (*current)["value"] = "deep";
        
        std::string serialized = deep.ToString();
        assert(!serialized.empty());
    }, false);
    
    tester.add_test("Serialize very large array", []() {
        auto large_arr = Json::Array();
        for (int i = 0; i < 10000; ++i) {
            large_arr.PushBack(i);
        }
        
        std::string serialized = large_arr.ToString();
        assert(!serialized.empty());
    }, false);
    
    tester.add_test("Serialize special characters", []() {
        Json special = "String with \"quotes\" and \\backslashes\\ and \nnewlines\n";
        std::string serialized = special.ToString();
        assert(!serialized.empty());
        
        // Should be able to parse it back
        auto parsed = Json::Parse(serialized);
        assert(parsed.Get<std::string>() == special.Get<std::string>());
    }, false);
    
    tester.add_test("Serialize unicode", []() {
        Json unicode = "Hello 世界 🌍 café naïve résumé";
        std::string serialized = unicode.ToString();
        assert(!serialized.empty());
    }, false);
    
    // Test potential circular references (if the library supports them)
    tester.add_test("Detect circular reference", []() {
        auto obj1 = Json::Object();
        obj1["self"] = obj1;  // Creates a true circular reference
        
        // This should throw an exception (circular reference detected)
        try {
            std::string serialized = obj1.ToString();
            // If we get here, cycles were not detected - this is unexpected
        } catch (const JsonException&) {
            // If we get here, cycles are detected and rejected
            throw;  // Re-throw to indicate this is the expected behavior
        }
    }, true, "JsonException");  // Expecting this to throw
    
    tester.run_all_tests();
}

void test_memory_and_resource_errors() {
    std::cout << "\n=== Memory and Resource Error Tests ===\n";
    
    ErrorTester tester;
    
    // Test self-assignment
    tester.add_test("Self assignment safety", []() {
        auto obj = Json::Object();
        obj["key"] = "value";
        obj = obj;  // Self-assignment
        assert(obj["key"].Get<std::string>() == "value");
    }, false);
    
    // Test move after move
    tester.add_test("Move after move", []() {
        auto obj1 = Json::Object();
        obj1["key"] = "value";
        auto obj2 = std::move(obj1);
        auto obj3 = std::move(obj2);
        assert(obj3["key"].Get<std::string>() == "value");
    }, false);
    
    // Test accessing moved-from object (behavior may be undefined, but shouldn't crash)
    // RE-ENABLED TO DEBUG SEGFAULT ISSUE
    tester.add_test("Access moved-from object - DEBUG", []() {
        std::cout << "Creating original object..." << std::endl;
        auto obj1 = Json::Object();
        obj1["key"] = "value";
        std::cout << "Original object created with key-value pair" << std::endl;
        
        std::cout << "Moving object..." << std::endl;
        auto obj2 = std::move(obj1);
        std::cout << "Object moved successfully" << std::endl;
        
        std::cout << "About to access moved-from object..." << std::endl;
        // Accessing obj1 after move - this is where the segfault occurs
        try {
            std::cout << "Calling IsObject() on moved-from object..." << std::endl;
            bool is_object = obj1.IsObject();
            std::cout << "IsObject() returned: " << is_object << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Exception caught: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "Unknown exception caught" << std::endl;
        }
        std::cout << "Moved-from object access completed" << std::endl;
    }, false);
    
    // Test exception safety during construction
    tester.add_test("Exception safety in copy constructor", []() {
        auto original = Json::Object();
        for (int i = 0; i < 1000; ++i) {
            original["key" + std::to_string(i)] = "value" + std::to_string(i);
        }
        
        // This should not throw under normal circumstances
        auto copied = original;
        assert(copied.Keys().size() == 1000);
    }, false);
    
    tester.run_all_tests();
}

void test_safe_access_methods() {
    std::cout << "\n=== Safe Access Method Tests ===\n";
    
    ErrorTester tester;
    
    // TryGet should never throw, only return nullopt for invalid conversions
    tester.add_test("TryGet with wrong type returns nullopt", []() {
        Json str = "hello";
        auto result = str.TryGet<int>();
        assert(!result.has_value());
    }, false);
    
    tester.add_test("TryGet with correct type returns value", []() {
        Json num = 42;
        auto result = num.TryGet<int>();
        assert(result.has_value() && result.value() == 42);
    }, false);
    
    tester.add_test("TryGet on null returns nullopt", []() {
        Json null_val = nullptr;
        auto result = null_val.TryGet<std::string>();
        assert(!result.has_value());
    }, false);
    
    // Contains should never throw on objects
    tester.add_test("Contains on object with existing key", []() {
        Json obj = Json::Object();
        obj["key"] = "value";
        assert(obj.Contains("key") == true);
    }, false);
    
    tester.add_test("Contains on object with non-existing key", []() {
        Json obj = Json::Object();
        assert(obj.Contains("nonexistent") == false);
    }, false);
    
    // Type checking methods should never throw
    tester.add_test("Type checking methods never throw", []() {
        std::vector<Json> values = {
            nullptr,
            true,
            42,
            3.14,
            "string",
            Json::Array(),
            Json::Object()
        };
        
        for (const auto& val : values) {
            val.IsNull();
            val.IsBoolean();
            val.IsNumber();
            val.IsString();
            val.IsArray();
            val.IsObject();
            val.GetType();
        }
    }, false);
    
    tester.run_all_tests();
}

int main() {
    try {
        std::cout << "JSON Library Error Handling Test Suite\n";
        std::cout << "======================================\n";
        
        test_type_errors();
        test_boundary_conditions();
        test_parsing_errors();
        test_serialization_errors();
        test_memory_and_resource_errors();
        test_safe_access_methods();
        
        std::cout << "\n🎯 Error handling test suite completed!\n";
        std::cout << "This suite tests that the library properly handles error conditions\n";
        std::cout << "and throws appropriate exceptions when expected.\n";
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error handling test suite failed with exception: " << e.what() << std::endl;
        return 1;
    }
} 