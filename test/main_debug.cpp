#include "../Json.h"
#include <iostream>
#include <cassert>

void testSerialization() {
    std::cout << "=== Testing Serialization ===\n";
    
    std::cout << "Creating main object...\n";
    auto json = Json::Object();
    
    std::cout << "Adding name...\n";
    json["name"] = "Bob";
    
    std::cout << "Creating details object...\n";
    json["details"] = Json::Object();
    
    std::cout << "Adding age to details...\n";
    json["details"]["age"] = 35;
    
    std::cout << "Creating hobbies array...\n";
    json["details"]["hobbies"] = Json::Array();
    
    std::cout << "Adding reading hobby...\n";
    json["details"]["hobbies"].PushBack("reading");
    
    std::cout << "Adding swimming hobby...\n";
    json["details"]["hobbies"].PushBack("swimming");
    
    std::cout << "Adding active field...\n";
    json["active"] = true;
    
    std::cout << "Adding balance field...\n";
    json["balance"] = 1234.56;
    
    std::cout << "Adding metadata field...\n";
    json["metadata"] = nullptr;
    
    std::cout << "About to call ToString()...\n";
    // Compact serialization
    std::cout << "Compact JSON:\n" << json.ToString() << std::endl;
    
    std::cout << "About to call ToString(true)...\n";
    // Pretty serialization
    std::cout << "\nPretty JSON:\n" << json.ToString(true) << std::endl;
    
    std::cout << "Serialization tests passed!\n\n";
}

int main() {
    try {
        std::cout << "Starting serialization debug test...\n";
        testSerialization();
        std::cout << "Test completed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 