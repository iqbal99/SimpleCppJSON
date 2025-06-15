#include "../Json.h"
#include <iostream>

int main() {
    try {
        std::cout << "Creating Json object...\n";
        auto json = Json::Object();
        
        std::cout << "Adding values...\n";
        json["name"] = "Alice";
        json["age"] = 25;
        
        std::cout << "Getting object range...\n";
        auto range = json.ObjectItems();
        
        std::cout << "Getting begin iterator...\n";
        auto it = range.begin();
        
        std::cout << "Getting end iterator...\n";
        auto end = range.end();
        
        std::cout << "Checking if iterator is valid...\n";
        if (it != end) {
            std::cout << "Attempting to dereference iterator...\n";
            auto item = *it;
            std::cout << "Key: " << item.key << std::endl;
            std::cout << "Value: " << item.value().ToString() << std::endl;
        } else {
            std::cout << "Iterator is empty\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 