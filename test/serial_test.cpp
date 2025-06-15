#include "../Json.h"
#include <iostream>

int main() {
    try {
        std::cout << "Creating Json object...\n";
        auto json = Json::Object();
        
        std::cout << "Adding name...\n";
        json["name"] = "Bob";
        
        std::cout << "Creating details object...\n";
        json["details"] = Json::Object();
        
        std::cout << "Adding age to details...\n";
        json["details"]["age"] = 35;
        
        std::cout << "Creating hobbies array...\n";
        json["details"]["hobbies"] = Json::Array();
        
        std::cout << "Adding hobbies...\n";
        json["details"]["hobbies"].PushBack("reading");
        json["details"]["hobbies"].PushBack("swimming");
        
        std::cout << "Adding other fields...\n";
        json["active"] = true;
        json["balance"] = 1234.56;
        json["metadata"] = nullptr;
        
        std::cout << "Testing compact serialization...\n";
        std::string compact = json.ToString();
        std::cout << "Compact JSON:\n" << compact << std::endl;
        
        std::cout << "Testing pretty serialization...\n";
        std::string pretty = json.ToString(true);
        std::cout << "Pretty JSON:\n" << pretty << std::endl;
        
        std::cout << "Serialization test completed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 