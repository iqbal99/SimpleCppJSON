#include "../Json.h"
#include <iostream>

int main() {
    std::cout << "=== MINIMAL REPRODUCTION OF CRITICAL ISSUES ===\n\n";
    
    // Issue #1: Moved-from object segfault
    std::cout << "1. Testing moved-from object access (WILL SEGFAULT):\n";
    std::cout << "   Creating object...\n";
    auto obj = Json::Object();
    obj["test"] = 123;
    
    std::cout << "   Moving object...\n"; 
    auto moved = std::move(obj);
    
    std::cout << "   About to access moved-from object (this will crash)...\n";
    std::cout << "   obj.IsObject() = ";
    std::cout.flush();
    
    try {
        bool result = obj.IsObject();  // <-- SEGFAULT HERE
        std::cout << result << " (unexpected success!)\n";
    } catch (...) {
        std::cout << "Exception caught (good!)\n";
    }
    
    // If we get here, the segfault was fixed
    std::cout << "\n2. Testing ObjectItems() on non-object (WILL CRASH):\n";
    Json str = "hello";
    std::cout << "   About to call ObjectItems() on string...\n";
    std::cout.flush();
    
    try {
        auto range = str.ObjectItems();
        auto it = range.begin();  // <-- bad_variant_access HERE
        std::cout << "   ObjectItems() succeeded (unexpected!)\n";
    } catch (const std::exception& e) {
        std::cout << "   Exception: " << e.what() << "\n";
    }
    
    std::cout << "\n3. Testing other problematic operations:\n";
    
    Json number = 42;
    std::cout << "   Size() on number: " << number.Size() << " (should throw?)\n";
    std::cout << "   Contains() on number: " << number.Contains("key") << " (should throw?)\n";
    
    try {
        auto keys = number.Keys();
        std::cout << "   Keys() on number: returned " << keys.size() << " keys (unexpected!)\n";
    } catch (const std::exception& e) {
        std::cout << "   Keys() on number threw: " << e.what() << "\n";
    }
    
    std::cout << "\nAll tests completed without crashing - issues may be fixed!\n";
    return 0;
} 