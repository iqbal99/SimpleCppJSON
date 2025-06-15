#include "../Json.h"
#include <iostream>
#include <cassert>

int main() {
    try {
        std::cout << "Creating Json object...\n";
        auto json = Json::Object();
        
        std::cout << "Adding values...\n";
        json["name"] = "Alice";
        json["age"] = 25;
        json["city"] = "New York";
        json["active"] = true;
        
        std::cout << "Checking Contains for 'city'...\n";
        bool hasCity = json.Contains("city");
        std::cout << "Contains city: " << (hasCity ? "true" : "false") << std::endl;
        
        std::cout << "Removing 'city'...\n";
        json.Remove("city");
        
        std::cout << "Checking Contains for 'city' again...\n";
        bool hasCityAfter = json.Contains("city");
        std::cout << "Contains city after removal: " << (hasCityAfter ? "true" : "false") << std::endl;
        
        std::cout << "Test completed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 