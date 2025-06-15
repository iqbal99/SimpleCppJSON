#include "../Json.h"
#include <iostream>
#include <cassert>

void testBasicFunctionality() {
    std::cout << "=== Testing Basic Functionality ===\n";
    
    // Create an object
    auto json = Json::Object();
    
    // Add some values
    json["name"] = "John Doe";
    json["age"] = 30;
    json["height"] = 5.9;
    json["married"] = true;
    json["spouse"] = nullptr;
    
    // Test type checking
    assert(json["name"].IsString());
    assert(json["age"].IsNumber());
    assert(json["height"].IsNumber());
    assert(json["married"].IsBoolean());
    assert(json["spouse"].IsNull());
    
    // Access values
    std::string name = json["name"].Get<std::string>();
    int age = json["age"].Get<int>();
    double height = json["height"].Get<double>();
    bool married = json["married"].Get<bool>();
    
    std::cout << "Name: " << name << std::endl;
    std::cout << "Age: " << age << std::endl;
    std::cout << "Height: " << height << std::endl;
    std::cout << "Married: " << (married ? "true" : "false") << std::endl;
    
    // Test safe access
    auto email = json["email"].TryGet<std::string>();
    if (email) {
        std::cout << "Email: " << *email << std::endl;
    } else {
        std::cout << "No email provided" << std::endl;
    }
    
    std::cout << "Basic functionality tests passed!\n\n";
}

void testArrays() {
    std::cout << "=== Testing Arrays ===\n";
    
    auto json = Json::Object();
    json["scores"] = Json::Array();
    json["scores"].PushBack(95);
    json["scores"].PushBack(87);
    json["scores"].PushBack(91);
    json["scores"].PushBack(88.5);
    
    std::cout << "Array size: " << json["scores"].Size() << std::endl;
    
    // Iterate over array using range-based for
    std::cout << "Scores: ";
    for (const auto& score : json["scores"]) {
        std::cout << score.Get<double>() << " ";
    }
    std::cout << std::endl;
    
    // Test array indexing
    std::cout << "First score: " << json["scores"][0].Get<int>() << std::endl;
    std::cout << "Last score: " << json["scores"][3].Get<double>() << std::endl;
    
    std::cout << "Array tests passed!\n\n";
}

void testObjectIteration() {
    std::cout << "=== Testing Object Iteration ===\n";
    
    auto json = Json::Object();
    json["name"] = "Alice";
    json["age"] = 25;
    json["city"] = "New York";
    json["active"] = true;
    
    // Iterate over object using structured bindings
    std::cout << "Object contents:\n";
    for (const auto& item : json.ObjectItems()) {
        std::cout << "  " << item.key << ": " << item.value().ToString() << std::endl;
    }
    
    // Test Contains and Remove
    assert(json.Contains("city"));
    json.Remove("city");
    assert(!json.Contains("city"));
    
    std::cout << "Object iteration tests passed!\n\n";
}

void testSerialization() {
    std::cout << "=== Testing Serialization ===\n";
    
    auto json = Json::Object();
    json["name"] = "Bob";
    json["details"] = Json::Object();
    json["details"]["age"] = 35;
    json["details"]["hobbies"] = Json::Array();
    json["details"]["hobbies"].PushBack("reading");
    json["details"]["hobbies"].PushBack("swimming");
    json["active"] = true;
    json["balance"] = 1234.56;
    json["metadata"] = nullptr;
    
    // Compact serialization
    std::cout << "Compact JSON:\n" << json.ToString() << std::endl;
    
    // Pretty serialization
    std::cout << "\nPretty JSON:\n" << json.ToString(true) << std::endl;
    
    std::cout << "Serialization tests passed!\n\n";
}

void testParsing() {
    std::cout << "=== Testing JSON Parsing ===\n";
    
    // Test parsing a simple object
    auto parsed = Json::Parse(R"({
        "name": "Jane Doe",
        "age": 28,
        "scores": [98, 95, 92],
        "active": true,
        "metadata": null,
        "height": 5.6
    })");
    
    assert(parsed.IsObject());
    assert(parsed["name"].Get<std::string>() == "Jane Doe");
    assert(parsed["age"].Get<int>() == 28);
    assert(parsed["active"].Get<bool>() == true);
    assert(parsed["metadata"].IsNull());
    assert(parsed["height"].Get<double>() == 5.6);
    assert(parsed["scores"].IsArray());
    assert(parsed["scores"].Size() == 3);
    
    std::cout << "Parsed JSON:\n" << parsed.ToString(true) << std::endl;
    
    // Test parsing an array
    auto arrayJson = Json::Parse("[1, 2, 3, \"hello\", true, null]");
    assert(arrayJson.IsArray());
    assert(arrayJson.Size() == 6);
    assert(arrayJson[0].Get<int>() == 1);
    assert(arrayJson[3].Get<std::string>() == "hello");
    assert(arrayJson[4].Get<bool>() == true);
    assert(arrayJson[5].IsNull());
    
    std::cout << "Parsed array: " << arrayJson.ToString() << std::endl;
    
    std::cout << "Parsing tests passed!\n\n";
}

void testErrorHandling() {
    std::cout << "=== Testing Error Handling ===\n";
    
    try {
        // Type error
        auto json = Json("hello");
        int value = json.Get<int>(); // Should throw
        (void)value; // Suppress unused variable warning
        assert(false && "Should have thrown");
    } catch (const JsonTypeError& e) {
        std::cout << "Caught expected type error: " << e.what() << std::endl;
    }
    
    try {
        // Parse error
        auto json = Json::Parse("{invalid json}");
        assert(false && "Should have thrown");
    } catch (const JsonParseError& e) {
        std::cout << "Caught expected parse error: " << e.what() << std::endl;
    }
    
    try {
        // Array bounds error
        auto json = Json::Array();
        auto& item = json[0]; // Should throw
        (void)item; // Suppress unused variable warning
        assert(false && "Should have thrown");
    } catch (const JsonException& e) {
        std::cout << "Caught expected bounds error: " << e.what() << std::endl;
    }
    
    std::cout << "Error handling tests passed!\n\n";
}

void testAdvancedFeatures() {
    std::cout << "=== Testing Advanced Features ===\n";
    
    // Test copy and move semantics
    auto original = Json::Object();
    original["data"] = "important";
    
    auto copied = original;
    auto moved = std::move(original);
    
    assert(copied["data"].Get<std::string>() == "important");
    assert(moved["data"].Get<std::string>() == "important");
    
    // Test nested structures
    auto complex = Json::Object();
    complex["users"] = Json::Array();
    
    for (int i = 0; i < 3; ++i) {
        auto user = Json::Object();
        user["id"] = i;
        user["name"] = "User " + std::to_string(i);
        user["active"] = (i % 2 == 0);
        complex["users"].PushBack(std::move(user));
    }
    
    std::cout << "Complex nested structure:\n" << complex.ToString(true) << std::endl;
    
    // Test keys retrieval
    auto keys = complex.Keys();
    std::cout << "Top-level keys: ";
    for (const auto& key : keys) {
        std::cout << key << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Advanced features tests passed!\n\n";
}

int main() {
    try {
        std::cout << "JSON Library Basic Comprehensive Test Suite\n";
        std::cout << "==========================================\n\n";
        
        testBasicFunctionality();
        testArrays();
        testObjectIteration();
        testSerialization();
        testParsing();
        testErrorHandling();
        testAdvancedFeatures();
        
        std::cout << "🎉 All basic comprehensive tests passed successfully!\n";
        std::cout << "For more extensive testing, run:\n";
        std::cout << "  make test-advanced  # Advanced scenarios and edge cases\n";
        std::cout << "  make test-stress    # Performance and stress testing\n";
        std::cout << "  make test-errors    # Error handling and boundary conditions\n";
        std::cout << "  make test-iterators # Comprehensive iterator testing\n";
        std::cout << "  make test           # Run all tests\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}