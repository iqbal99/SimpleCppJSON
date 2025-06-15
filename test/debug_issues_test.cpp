#include "../Json.h"
#include <iostream>
#include <cassert>
#include <string>
#include <variant>

void debug_object_iteration_on_non_objects() {
    std::cout << "\n=== DEBUG: Object Iteration on Non-Objects ===\n";
    
    // Test with different types
    std::vector<std::pair<std::string, Json>> test_cases = {
        {"null", Json(nullptr)},
        {"boolean", Json(true)},
        {"integer", Json(42)},
        {"double", Json(3.14)},
        {"string", Json("hello")},
        {"array", Json::Array()}
    };
    
    for (const auto& [type_name, json_val] : test_cases) {
        std::cout << "\n--- Testing ObjectItems() on " << type_name << " ---" << std::endl;
        std::cout << "Type: " << static_cast<int>(json_val.GetType()) << std::endl;
        std::cout << "IsObject(): " << json_val.IsObject() << std::endl;
        
        try {
            std::cout << "About to call ObjectItems()..." << std::endl;
            auto range = json_val.ObjectItems();
            std::cout << "ObjectItems() call succeeded, getting begin()..." << std::endl;
            
            auto begin_it = range.begin();
            std::cout << "begin() call succeeded, getting end()..." << std::endl;
            
            auto end_it = range.end();
            std::cout << "end() call succeeded, starting iteration..." << std::endl;
            
            int count = 0;
            for (auto it = begin_it; it != end_it; ++it) {
                std::cout << "Iterator " << count << ": about to dereference..." << std::endl;
                auto item = *it;
                std::cout << "Dereferenced successfully, key: " << item.key << std::endl;
                count++;
                if (count > 10) {
                    std::cout << "Breaking after 10 iterations to prevent infinite loop" << std::endl;
                    break;
                }
            }
            std::cout << "Iteration completed successfully, count: " << count << std::endl;
            
        } catch (const std::bad_variant_access& e) {
            std::cout << "❌ bad_variant_access: " << e.what() << std::endl;
            std::cout << "This suggests the library is trying to access the wrong variant type" << std::endl;
        } catch (const JsonException& e) {
            std::cout << "✓ JsonException (expected): " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cout << "❌ std::exception: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "❌ Unknown exception caught" << std::endl;
        }
    }
}

void debug_moved_from_object_access() {
    std::cout << "\n=== DEBUG: Moved-From Object Access ===\n";
    
    // Test different operations on moved-from objects
    std::cout << "Creating original object..." << std::endl;
    auto original = Json::Object();
    original["key1"] = "value1";
    original["key2"] = 42;
    original["key3"] = Json::Array();
    
    std::cout << "Original object created successfully" << std::endl;
    std::cout << "Original IsObject(): " << original.IsObject() << std::endl;
    std::cout << "Original size: " << original.Keys().size() << std::endl;
    
    std::cout << "Moving object..." << std::endl;
    auto moved_to = std::move(original);
    std::cout << "Move completed successfully" << std::endl;
    
    std::cout << "Moved-to object IsObject(): " << moved_to.IsObject() << std::endl;
    std::cout << "Moved-to object size: " << moved_to.Keys().size() << std::endl;
    
    // Now test various operations on the moved-from object
    std::vector<std::string> operations = {
        "IsObject", "IsNull", "IsString", "IsArray", "IsBoolean", "IsNumber",
        "GetType", "Size", "Keys", "Contains", "ToString"
    };
    
    for (const auto& op : operations) {
        std::cout << "\n--- Testing " << op << "() on moved-from object ---" << std::endl;
        try {
            if (op == "IsObject") {
                bool result = original.IsObject();
                std::cout << "✓ " << op << "() returned: " << result << std::endl;
            } else if (op == "IsNull") {
                bool result = original.IsNull();
                std::cout << "✓ " << op << "() returned: " << result << std::endl;
            } else if (op == "IsString") {
                bool result = original.IsString();
                std::cout << "✓ " << op << "() returned: " << result << std::endl;
            } else if (op == "IsArray") {
                bool result = original.IsArray();
                std::cout << "✓ " << op << "() returned: " << result << std::endl;
            } else if (op == "IsBoolean") {
                bool result = original.IsBoolean();
                std::cout << "✓ " << op << "() returned: " << result << std::endl;
            } else if (op == "IsNumber") {
                bool result = original.IsNumber();
                std::cout << "✓ " << op << "() returned: " << result << std::endl;
            } else if (op == "GetType") {
                auto result = original.GetType();
                std::cout << "✓ " << op << "() returned: " << static_cast<int>(result) << std::endl;
            } else if (op == "Size") {
                auto result = original.Size();
                std::cout << "✓ " << op << "() returned: " << result << std::endl;
            } else if (op == "Keys") {
                auto result = original.Keys();
                std::cout << "✓ " << op << "() returned vector with " << result.size() << " elements" << std::endl;
            } else if (op == "Contains") {
                bool result = original.Contains("key1");
                std::cout << "✓ " << op << "(\"key1\") returned: " << result << std::endl;
            } else if (op == "ToString") {
                std::string result = original.ToString();
                std::cout << "✓ " << op << "() returned string of length: " << result.length() << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "❌ " << op << "() threw exception: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "❌ " << op << "() threw unknown exception" << std::endl;
        }
    }
}

void debug_operations_on_wrong_types() {
    std::cout << "\n=== DEBUG: Operations on Wrong Types ===\n";
    
    // Test operations that should throw but might not
    Json number = 42;
    Json string_val = "hello";
    Json boolean = true;
    Json array = Json::Array();
    
    std::cout << "\n--- Testing Size() on different types ---" << std::endl;
    std::vector<std::pair<std::string, Json*>> types = {
        {"number", &number},
        {"string", &string_val}, 
        {"boolean", &boolean}
    };
    
    for (const auto& [name, json_ptr] : types) {
        std::cout << "Size() on " << name << ": ";
        try {
            size_t size = json_ptr->Size();
            std::cout << size << " (should this throw?)" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "threw " << e.what() << std::endl;
        }
    }
    
    std::cout << "\n--- Testing Contains() on different types ---" << std::endl;
    for (const auto& [name, json_ptr] : types) {
        std::cout << "Contains(\"key\") on " << name << ": ";
        try {
            bool result = json_ptr->Contains("key");
            std::cout << result << " (should this throw?)" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "threw " << e.what() << std::endl;
        }
    }
    
    std::cout << "\n--- Testing Keys() on different types ---" << std::endl;
    for (const auto& [name, json_ptr] : types) {
        std::cout << "Keys() on " << name << ": ";
        try {
            auto keys = json_ptr->Keys();
            std::cout << "returned " << keys.size() << " keys (should this throw?)" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "threw " << e.what() << std::endl;
        }
    }
    
    std::cout << "\n--- Testing array operations on non-arrays ---" << std::endl;
    for (const auto& [name, json_ptr] : types) {
        std::cout << "PushBack(1) on " << name << ": ";
        try {
            json_ptr->PushBack(1);
            std::cout << "succeeded (should this throw?)" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "threw " << e.what() << std::endl;
        }
    }
    
    std::cout << "\n--- Testing array index access on non-arrays ---" << std::endl;
    for (const auto& [name, json_ptr] : types) {
        std::cout << "operator[0] on " << name << ": ";
        try {
            auto& item = (*json_ptr)[0];
            std::cout << "succeeded, got type " << static_cast<int>(item.GetType()) << " (should this throw?)" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "threw " << e.what() << std::endl;
        }
    }
    
    std::cout << "\n--- Testing object key access on non-objects ---" << std::endl;
    std::cout << "operator[\"key\"] on array: ";
    try {
        auto& item = array["key"];
        std::cout << "succeeded, got type " << static_cast<int>(item.GetType()) << " (should this throw?)" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "threw " << e.what() << std::endl;
    }
}

void debug_circular_references() {
    std::cout << "\n=== DEBUG: Circular References ===\n";
    
    std::cout << "Creating circular reference..." << std::endl;
    auto obj1 = Json::Object();
    auto obj2 = Json::Object();
    
    obj1["name"] = "object1";
    obj2["name"] = "object2";
    
    std::cout << "Setting up circular references..." << std::endl;
    obj1["ref"] = obj2;
    obj2["backref"] = obj1;
    
    std::cout << "Circular reference created successfully" << std::endl;
    
    std::cout << "Testing serialization of circular reference..." << std::endl;
    try {
        std::string result = obj1.ToString();
        std::cout << "✓ Serialization succeeded, length: " << result.length() << std::endl;
        if (result.length() < 1000) {
            std::cout << "Result: " << result << std::endl;
        } else {
            std::cout << "Result too long to display (possible infinite expansion)" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "❌ Serialization threw: " << e.what() << std::endl;
    }
    
    std::cout << "Testing pretty serialization..." << std::endl;
    try {
        std::string result = obj1.ToString(true);
        std::cout << "✓ Pretty serialization succeeded, length: " << result.length() << std::endl;
        if (result.length() < 1000) {
            std::cout << "Result: " << result << std::endl;
        } else {
            std::cout << "Result too long to display (possible infinite expansion)" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "❌ Pretty serialization threw: " << e.what() << std::endl;
    }
}

int main() {
    try {
        std::cout << "JSON Library Debug Issues Test\n";
        std::cout << "============================\n";
        
        std::cout << "\nThis test will deliberately trigger known issues to help debug the JSON library.\n";
        std::cout << "Expect crashes, exceptions, and strange behavior.\n\n";
        
        debug_operations_on_wrong_types();
        debug_circular_references();
        debug_moved_from_object_access();  // This might segfault
        debug_object_iteration_on_non_objects();  // This might cause bad_variant_access
        
        std::cout << "\n🔍 Debug test completed successfully!\n";
        std::cout << "If you see this message, the library handled all edge cases gracefully.\n";
        
        return 0;
        
    } catch (const std::bad_variant_access& e) {
        std::cerr << "\n💥 CRITICAL: bad_variant_access caught: " << e.what() << std::endl;
        std::cerr << "This indicates the library is accessing the wrong variant type internally." << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "\n💥 CRITICAL: Exception caught: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n💥 CRITICAL: Unknown exception caught" << std::endl;
        return 1;
    }
} 