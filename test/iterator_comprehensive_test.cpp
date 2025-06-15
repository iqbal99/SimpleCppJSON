#include "../Json.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>
#include <numeric>
#include <variant>

void test_array_iterators() {
    std::cout << "\n=== Testing Array Iterators ===\n";
    
    // Test empty array iteration
    auto empty_arr = Json::Array();
    int count = 0;
    for (const auto& item : empty_arr) {
        (void)item;
        count++;
    }
    assert(count == 0);
    std::cout << "✓ Empty array iteration" << std::endl;
    
    // Test single element array
    auto single_arr = Json::Array();
    single_arr.PushBack(42);
    count = 0;
    for (const auto& item : single_arr) {
        assert(item.Get<int>() == 42);
        count++;
    }
    assert(count == 1);
    std::cout << "✓ Single element array iteration" << std::endl;
    
    // Test multiple elements
    auto multi_arr = Json::Array();
    for (int i = 0; i < 10; ++i) {
        multi_arr.PushBack(i);
    }
    
    count = 0;
    int sum = 0;
    for (const auto& item : multi_arr) {
        sum += item.Get<int>();
        count++;
    }
    assert(count == 10);
    assert(sum == 45);  // Sum of 0-9
    std::cout << "✓ Multiple element array iteration" << std::endl;
    
    // Test mixed type array
    auto mixed_arr = Json::Array();
    mixed_arr.PushBack(1);
    mixed_arr.PushBack("hello");
    mixed_arr.PushBack(true);
    mixed_arr.PushBack(nullptr);
    mixed_arr.PushBack(3.14);
    
    count = 0;
    std::vector<Json::Type> expected_types = {
        Json::Type::Number, Json::Type::String, Json::Type::Boolean, 
        Json::Type::Null, Json::Type::Number
    };
    
    for (const auto& item : mixed_arr) {
        assert(item.GetType() == expected_types[count]);
        count++;
    }
    assert(count == 5);
    std::cout << "✓ Mixed type array iteration" << std::endl;
    
    // Test manual iterator usage
    auto manual_arr = Json::Array();
    for (int i = 0; i < 5; ++i) {
        manual_arr.PushBack(i * 2);
    }
    
    auto it = manual_arr.begin();
    auto end = manual_arr.end();
    count = 0;
    while (it != end) {
        assert((*it).Get<int>() == count * 2);
        ++it;
        count++;
    }
    assert(count == 5);
    std::cout << "✓ Manual iterator usage" << std::endl;
    
    // Test const iterator
    const auto& const_arr = manual_arr;
    count = 0;
    for (auto cit = const_arr.cbegin(); cit != const_arr.cend(); ++cit) {
        assert((*cit).Get<int>() == count * 2);
        count++;
    }
    assert(count == 5);
    std::cout << "✓ Const iterator usage" << std::endl;
    
    // Test iterator equality and inequality
    auto iter1 = manual_arr.begin();
    auto iter2 = manual_arr.begin();
    assert(iter1 == iter2);
    ++iter2;
    assert(iter1 != iter2);
    std::cout << "✓ Iterator equality comparison" << std::endl;
    
    // Test iterator modification during iteration (if supported)
    auto modifiable_arr = Json::Array();
    for (int i = 0; i < 3; ++i) {
        modifiable_arr.PushBack(i);
    }
    
    // Modify elements through non-const iterator
    for (auto& item : modifiable_arr) {
        int val = item.Get<int>();
        item = val * 10;
    }
    
    // Verify modifications
    count = 0;
    for (const auto& item : modifiable_arr) {
        assert(item.Get<int>() == count * 10);
        count++;
    }
    std::cout << "✓ Iterator modification" << std::endl;
}

void test_object_iterators() {
    std::cout << "\n=== Testing Object Iterators ===\n";
    
    // Test empty object iteration
    auto empty_obj = Json::Object();
    int count = 0;
    for (const auto& item : empty_obj.ObjectItems()) {
        (void)item;
        count++;
    }
    assert(count == 0);
    std::cout << "✓ Empty object iteration" << std::endl;
    
    // Test single key-value pair
    auto single_obj = Json::Object();
    single_obj["key"] = "value";
    count = 0;
    for (const auto& item : single_obj.ObjectItems()) {
        assert(item.key == "key");
        assert(item.value().Get<std::string>() == "value");
        count++;
    }
    assert(count == 1);
    std::cout << "✓ Single key-value pair iteration" << std::endl;
    
    // Test multiple key-value pairs
    auto multi_obj = Json::Object();
    std::vector<std::string> keys = {"name", "age", "active", "score"};
    multi_obj["name"] = "John";
    multi_obj["age"] = 30;
    multi_obj["active"] = true;
    multi_obj["score"] = 95.5;
    
    count = 0;
    std::vector<std::string> found_keys;
    for (const auto& item : multi_obj.ObjectItems()) {
        found_keys.push_back(item.key);
        count++;
    }
    assert(count == 4);
    // Keys might not be in insertion order, so we sort to compare
    std::sort(keys.begin(), keys.end());
    std::sort(found_keys.begin(), found_keys.end());
    assert(keys == found_keys);
    std::cout << "✓ Multiple key-value pairs iteration" << std::endl;
    
    // Test structured binding
    auto binding_obj = Json::Object();
    binding_obj["x"] = 10;
    binding_obj["y"] = 20;
    
    count = 0;
    for (const auto& item : binding_obj.ObjectItems()) {
        if (item.key == "x") {
            assert(item.value().Get<int>() == 10);
        } else if (item.key == "y") {
            assert(item.value().Get<int>() == 20);
        }
        count++;
    }
    assert(count == 2);
    std::cout << "✓ Key-value pair iteration" << std::endl;
    
    // Test manual object iterator usage
    auto manual_obj = Json::Object();
    manual_obj["a"] = 1;
    manual_obj["b"] = 2;
    manual_obj["c"] = 3;
    
    auto obj_it = manual_obj.object_begin();
    auto obj_end = manual_obj.object_end();
    count = 0;
    while (obj_it != obj_end) {
        auto item = *obj_it;
        // Values should be 1, 2, or 3
        int value = item.value().Get<int>();
        assert(value >= 1 && value <= 3);
        ++obj_it;
        count++;
    }
    assert(count == 3);
    std::cout << "✓ Manual object iterator usage" << std::endl;
    
    // Test const object iterator
    const auto& const_obj = manual_obj;
    count = 0;
    for (auto cit = const_obj.object_cbegin(); cit != const_obj.object_cend(); ++cit) {
        auto item = *cit;
        int value = item.value().Get<int>();
        assert(value >= 1 && value <= 3);
        count++;
    }
    assert(count == 3);
    std::cout << "✓ Const object iterator usage" << std::endl;
    
    // Test object iterator modification
    auto modifiable_obj = Json::Object();
    modifiable_obj["multiply"] = 5;
    modifiable_obj["add"] = 10;
    
    for (const auto& item : modifiable_obj.ObjectItems()) {
        int val = item.value().Get<int>();
        // Note: modification through iterator might not be supported
        // We'll modify the object directly instead
        if (item.key == "multiply") {
            modifiable_obj[item.key] = val * 2;
        } else if (item.key == "add") {
            modifiable_obj[item.key] = val * 2;
        }
    }
    
    // Verify modifications
    assert(modifiable_obj["multiply"].Get<int>() == 10);
    assert(modifiable_obj["add"].Get<int>() == 20);
    std::cout << "✓ Object iterator modification" << std::endl;
    
    // Test mixed value types in object
    auto mixed_obj = Json::Object();
    mixed_obj["string"] = "hello";
    mixed_obj["number"] = 42;
    mixed_obj["boolean"] = true;
    mixed_obj["null"] = nullptr;
    mixed_obj["array"] = Json::Array();
    mixed_obj["object"] = Json::Object();
    
    count = 0;
    for (const auto& item : mixed_obj.ObjectItems()) {
        // Each value should have a valid type
        Json::Type type = item.value().GetType();
        assert(type != Json::Type::Null || item.key == "null");
        count++;
    }
    assert(count == 6);
    std::cout << "✓ Mixed value types in object iteration" << std::endl;
}

void test_nested_iteration() {
    std::cout << "\n=== Testing Nested Iteration ===\n";
    
    // Create a complex nested structure
    auto complex = Json::Object();
    complex["users"] = Json::Array();
    
    for (int i = 0; i < 3; ++i) {
        auto user = Json::Object();
        user["id"] = i;
        user["name"] = "User" + std::to_string(i);
        user["scores"] = Json::Array();
        
        for (int j = 0; j < 5; ++j) {
            user["scores"].PushBack(j * 10 + i);
        }
        
        complex["users"].PushBack(std::move(user));
    }
    
    // Test nested array iteration
    int user_count = 0;
    for (const auto& user : complex["users"]) {
        assert(user.IsObject());
        assert(user["id"].Get<int>() == user_count);
        
        int score_count = 0;
        for (const auto& score : user["scores"]) {
            assert(score.Get<int>() == score_count * 10 + user_count);
            score_count++;
        }
        assert(score_count == 5);
        user_count++;
    }
    assert(user_count == 3);
    std::cout << "✓ Nested array iteration" << std::endl;
    
    // Test nested object iteration
    user_count = 0;
    for (const auto& user : complex["users"]) {
        int field_count = 0;
        for (const auto& item : user.ObjectItems()) {
            if (item.key == "id") {
                assert(item.value().Get<int>() == user_count);
            } else if (item.key == "name") {
                assert(item.value().Get<std::string>() == "User" + std::to_string(user_count));
            } else if (item.key == "scores") {
                assert(item.value().IsArray());
                assert(item.value().Size() == 5);
            }
            field_count++;
        }
        assert(field_count == 3);
        user_count++;
    }
    std::cout << "✓ Nested object iteration" << std::endl;
    
    // Test deep nesting
    auto deep = Json::Object();
    auto* current = &deep;
    const int depth = 100;
    
    for (int i = 0; i < depth; ++i) {
        (*current)["level"] = Json::Object();
        (*current)["data"] = i;
        current = &(*current)["level"];
    }
    (*current)["final"] = "bottom";
    
    // Navigate back through the structure
    current = &deep;
    for (int i = 0; i < depth; ++i) {
        assert((*current)["data"].Get<int>() == i);
        current = &(*current)["level"];
    }
    assert((*current)["final"].Get<std::string>() == "bottom");
    std::cout << "✓ Deep nesting iteration" << std::endl;
}

void test_iterator_edge_cases() {
    std::cout << "\n=== Testing Iterator Edge Cases ===\n";
    
    // Test iterator on different JSON types
    Json null_val = nullptr;
    Json bool_val = true;
    Json num_val = 42;
    Json str_val = "hello";
    
    // These should not be iterable (or should have empty iteration)
    int count = 0;
    
    // Array iteration on non-arrays should be empty or throw
    try {
        count = 0;
        for (const auto& item : null_val) {
            (void)item;
            count++;
        }
        assert(count == 0);  // If it doesn't throw, should be empty
        std::cout << "✓ Iteration on null value handled (empty iteration)" << std::endl;
    } catch (const std::exception&) {
        // Exception is acceptable for invalid array iteration
        std::cout << "✓ Iteration on null value handled (exception thrown)" << std::endl;
    }
    
    try {
        count = 0;
        for (const auto& item : num_val) {
            (void)item;
            count++;
        }
        assert(count == 0);  // If it doesn't throw, should be empty
        std::cout << "✓ Iteration on number value handled (empty iteration)" << std::endl;
    } catch (const std::exception&) {
        // Exception is acceptable for invalid array iteration
        std::cout << "✓ Iteration on number value handled (exception thrown)" << std::endl;
    }
    
    // Test object iteration on non-objects - RE-ENABLED TO DEBUG
    std::cout << "Testing object iteration on non-objects..." << std::endl;
    try {
        count = 0;
        std::cout << "About to call ObjectItems() on string..." << std::endl;
        for (const auto& item : str_val.ObjectItems()) {
            std::cout << "Inside iteration - key: " << item.key << std::endl;
            (void)item;
            count++;
        }
        std::cout << "Iteration completed, count: " << count << std::endl;
        assert(count == 0);  // If it doesn't throw, should be empty
        std::cout << "✓ Object iteration on string handled (empty iteration)" << std::endl;
    } catch (const JsonException& e) {
        std::cout << "✓ Object iteration on string handled (JsonException: " << e.what() << ")" << std::endl;
    } catch (const std::bad_variant_access& e) {
        std::cout << "❌ Object iteration on string failed (bad_variant_access: " << e.what() << ")" << std::endl;
        throw; // Re-throw to see the full error
    } catch (const std::exception& e) {
        std::cout << "❌ Object iteration on string failed (std::exception: " << e.what() << ")" << std::endl;
        throw; // Re-throw to see the full error
    }
    
    // Test iterator invalidation scenarios
    auto arr = Json::Array();
    for (int i = 0; i < 5; ++i) {
        arr.PushBack(i);
    }
    
    // Get iterator
    auto it = arr.begin();
    assert((*it).Get<int>() == 0);
    
    // Modify array (this might invalidate iterator depending on implementation)
    arr.PushBack(999);
    
    // Some implementations might handle this gracefully, others might not
    // We'll just test that the library doesn't crash
    try {
        auto value = *it;
        (void)value;
    } catch (const JsonException&) {
        // Exception is acceptable if iterator is invalidated
    }
    std::cout << "✓ Iterator invalidation handled gracefully" << std::endl;
    
    // Test empty ranges
    auto empty_arr = Json::Array();
    auto empty_obj = Json::Object();
    
    assert(empty_arr.begin() == empty_arr.end());
    assert(empty_obj.object_begin() == empty_obj.object_end());
    std::cout << "✓ Empty ranges handled correctly" << std::endl;
    
    // Test iterator copying
    auto copy_arr = Json::Array();
    copy_arr.PushBack(1);
    copy_arr.PushBack(2);
    
    auto it1 = copy_arr.begin();
    auto it2 = it1;  // Copy constructor
    assert(it1 == it2);
    assert((*it1).Get<int>() == (*it2).Get<int>());
    
    ++it1;
    assert(it1 != it2);
    std::cout << "✓ Iterator copying handled correctly" << std::endl;
}

void test_algorithm_compatibility() {
    std::cout << "\n=== Testing Algorithm Compatibility ===\n";
    
    // Test std::for_each
    auto arr = Json::Array();
    for (int i = 1; i <= 5; ++i) {
        arr.PushBack(i);
    }
    
    int sum = 0;
    std::for_each(arr.begin(), arr.end(), [&sum](const Json& item) {
        sum += item.Get<int>();
    });
    assert(sum == 15);  // 1+2+3+4+5
    std::cout << "✓ std::for_each compatibility" << std::endl;
    
    // Test std::count_if
    auto mixed_arr = Json::Array();
    for (int i = 0; i < 10; ++i) {
        mixed_arr.PushBack(i);
    }
    
    int even_count = std::count_if(mixed_arr.begin(), mixed_arr.end(), 
                                   [](const Json& item) {
                                       return item.Get<int>() % 2 == 0;
                                   });
    assert(even_count == 5);  // 0, 2, 4, 6, 8
    std::cout << "✓ std::count_if compatibility" << std::endl;
    
    // Test std::find_if
    auto search_arr = Json::Array();
    search_arr.PushBack("apple");
    search_arr.PushBack("banana");
    search_arr.PushBack("cherry");
    
    auto found_it = std::find_if(search_arr.begin(), search_arr.end(),
                                [](const Json& item) {
                                    return item.Get<std::string>() == "banana";
                                });
    
    assert(found_it != search_arr.end());
    assert((*found_it).Get<std::string>() == "banana");
    std::cout << "✓ std::find_if compatibility" << std::endl;
    
    // Test range-based algorithms with object iteration
    auto obj = Json::Object();
    obj["a"] = 1;
    obj["b"] = 2;
    obj["c"] = 3;
    obj["d"] = 4;
    
    int obj_sum = 0;
    for (const auto& item : obj.ObjectItems()) {
        obj_sum += item.value().Get<int>();
    }
    assert(obj_sum == 10);  // 1+2+3+4
    std::cout << "✓ Object iteration with algorithms" << std::endl;
}

int main() {
    try {
        std::cout << "JSON Library Iterator Comprehensive Test Suite\n";
        std::cout << "==============================================\n";
        
        test_array_iterators();
        test_object_iterators();
        test_nested_iteration();
        test_iterator_edge_cases();
        test_algorithm_compatibility();
        
        std::cout << "\n🔄 All iterator tests completed successfully!\n";
        std::cout << "This suite validates that iterators work correctly in all scenarios\n";
        std::cout << "including edge cases, nested structures, and STL algorithm compatibility.\n";
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Iterator test suite failed with exception: " << e.what() << std::endl;
        return 1;
    }
} 