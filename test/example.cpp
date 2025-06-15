#include "../Json.h"
#include <iostream>

int main() {
    std::cout << "JSON Library Example\n";
    std::cout << "====================\n\n";

    // Creating JSON objects
    auto person = Json::Object();
    person["name"] = "Alice Johnson";
    person["age"] = 28;
    person["email"] = "alice@example.com";
    person["active"] = true;

    // Creating nested structures
    person["address"] = Json::Object();
    person["address"]["street"] = "123 Main St";
    person["address"]["city"] = "Springfield";
    person["address"]["zipcode"] = "12345";

    // Creating arrays
    person["hobbies"] = Json::Array();
    person["hobbies"].PushBack("reading");
    person["hobbies"].PushBack("photography");
    person["hobbies"].PushBack("hiking");

    // Adding numeric data
    person["scores"] = Json::Array();
    person["scores"].PushBack(85);
    person["scores"].PushBack(92);
    person["scores"].PushBack(78);

    std::cout << "Created person object:\n";
    std::cout << person.ToString(true) << std::endl;

    // Accessing data
    std::cout << "\nAccessing data:\n";
    std::cout << "Name: " << person["name"].Get<std::string>() << std::endl;
    std::cout << "Age: " << person["age"].Get<int>() << std::endl;
    std::cout << "City: " << person["address"]["city"].Get<std::string>() << std::endl;

    // Iterating over arrays
    std::cout << "\nHobbies: ";
    for (const auto& hobby : person["hobbies"]) {
        std::cout << hobby.Get<std::string>() << " ";
    }
    std::cout << std::endl;

    // Iterating over object properties
    std::cout << "\nAddress details:\n";
    for (const auto& item : person["address"].ObjectItems()) {
        std::cout << "  " << item.key << ": " << item.value().Get<std::string>() << std::endl;
    }

    // Parsing JSON from string
    std::cout << "\nParsing JSON from string:\n";
    std::string jsonString = R"({
        "product": "Laptop",
        "price": 999.99,
        "inStock": true,
        "categories": ["electronics", "computers"],
        "specifications": {
            "cpu": "Intel i7",
            "ram": "16GB",
            "storage": "512GB SSD"
        }
    })";

    auto product = Json::Parse(jsonString);
    std::cout << "Parsed product:\n";
    std::cout << product.ToString(true) << std::endl;

    // Demonstrating safe access
    std::cout << "\nSafe access example:\n";
    if (auto warranty = product["warranty"].TryGet<std::string>()) {
        std::cout << "Warranty: " << *warranty << std::endl;
    } else {
        std::cout << "No warranty information available" << std::endl;
    }

    return 0;
} 