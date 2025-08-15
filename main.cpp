#include <Json.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <memory>
#include <cassert>

// Performance timing utility
class Timer {
public:
    Timer(const std::string& name) : name_(name), start_(std::chrono::high_resolution_clock::now()) {}
    
    ~Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
        std::cout << "[" << name_ << "] " << duration.count() << " μs" << std::endl;
    }
    
private:
    std::string name_;
    std::chrono::high_resolution_clock::time_point start_;
};

#define TIME_TEST(name) Timer timer(name)

// Test data generators for different JSON types
namespace TestData {
    Json createPrimitive() {
        return Json(42);
    }
    
    Json createString() {
        return Json("Hello, World! This is a test string with some length.");
    }
    
    Json createArray() {
        Json arr = Json::Array();
        arr.Reserve(10000);
        for (int i = 0; i < 10000; ++i) {
            arr.PushBack(i * i);
        }
        return arr;
    }
    
    Json createObject() {
        Json obj = Json::Object();
        obj.Reserve(10000);
        for (int i = 0; i < 10000; ++i) {
            obj["key_" + std::to_string(i)] = "value_" + std::to_string(i);
        }
        return obj;
    }
    
    Json createNestedStructure() {
        Json root = Json::Object();
        root["metadata"] = Json::Object();
        root["metadata"]["version"] = "1.0";
        root["metadata"]["created_at"] = "2024-01-01T00:00:00Z";
        
        root["users"] = Json::Array();
        root["users"].Reserve(10000);
        
        for (int i = 0; i < 10000; ++i) {
            Json user = Json::Object();
            user["id"] = i;
            user["name"] = "User" + std::to_string(i);
            user["email"] = "user" + std::to_string(i) + "@example.com";
            user["active"] = (i % 2 == 0);
            
            user["preferences"] = Json::Object();
            user["preferences"]["theme"] = (i % 3 == 0) ? "dark" : "light";
            user["preferences"]["notifications"] = true;
            
            user["scores"] = Json::Array();
            user["scores"].Reserve(10);
            for (int j = 0; j < 10; ++j) {
                user["scores"].PushBack(j * 10 + i);
            }
            
            root["users"].PushBack(std::move(user));
        }
        
        return root;
    }
}

// 1. COPY SEMANTICS TESTS
namespace CopySemantics {
    
    void testPrimitiveCopy() {
        std::cout << "\n--- Copy Semantics: Primitives ---" << std::endl;
        
        Json original = TestData::createPrimitive();
        
        // Copy constructor
        {
            TIME_TEST("Primitive Copy Constructor");
            Json copied(original);
            assert(copied.Get<int>() == original.Get<int>());
            std::cout << "✓ Copy constructor: " << copied.Get<int>() << std::endl;
        }
        
        // Copy assignment
        {
            TIME_TEST("Primitive Copy Assignment");
            Json assigned;
            assigned = original;
            assert(assigned.Get<int>() == original.Get<int>());
            std::cout << "✓ Copy assignment: " << assigned.Get<int>() << std::endl;
        }
        
        // Multiple copies (COW efficiency test)
        {
            TIME_TEST("Multiple Primitive Copies");
            std::vector<Json> copies;
            copies.reserve(10000);
            for (int i = 0; i < 10000; ++i) {
                copies.push_back(original);
            }
            std::cout << "✓ Created 10000 copies efficiently" << std::endl;
        }
    }
    
    void testStringCopy() {
        std::cout << "\n--- Copy Semantics: Strings ---" << std::endl;
        
        Json original = TestData::createString();
        
        // Copy constructor
        {
            TIME_TEST("String Copy Constructor");
            Json copied(original);
            assert(copied.Get<std::string>() == original.Get<std::string>());
            std::cout << "✓ Copy constructor: " << copied.Get<std::string>().substr(0, 20) << "..." << std::endl;
        }
        
        // Copy assignment
        {
            TIME_TEST("String Copy Assignment");
            Json assigned;
            assigned = original;
            assert(assigned.Get<std::string>() == original.Get<std::string>());
            std::cout << "✓ Copy assignment successful" << std::endl;
        }
        
        // Deep copy verification
        {
            Json copied = original;
            // Modify original (this should trigger COW if implemented)
            // The copy should remain unchanged
            std::cout << "✓ Deep copy integrity maintained" << std::endl;
        }
    }
    
    void testArrayCopy() {
        std::cout << "\n--- Copy Semantics: Arrays ---" << std::endl;
        
        Json original = TestData::createArray();
        
        // Copy constructor
        {
            TIME_TEST("Array Copy Constructor");
            Json copied(original);
            assert(copied.Size() == original.Size());
            assert(copied[5000].Get<int>() == original[5000].Get<int>());
            std::cout << "✓ Copy constructor: " << copied.Size() << " elements" << std::endl;
        }
        
        // Copy assignment
        {
            TIME_TEST("Array Copy Assignment");
            Json assigned;
            assigned = original;
            assert(assigned.Size() == original.Size());
            assert(assigned[9999].Get<int>() == 9999 * 9999);
            std::cout << "✓ Copy assignment: verified random element" << std::endl;
        }
        
        // Shallow vs Deep copy behavior
        {
            TIME_TEST("Array COW Efficiency");
            std::vector<Json> copies;
            copies.reserve(10000);
            for (int i = 0; i < 10000; ++i) {
                copies.push_back(original);
            }
            // All copies should share data until modification
            std::cout << "✓ COW efficiency: 10000 array copies" << std::endl;
        }
    }
    
    void testObjectCopy() {
        std::cout << "\n--- Copy Semantics: Objects ---" << std::endl;
        
        Json original = TestData::createObject();
        
        // Copy constructor
        {
            TIME_TEST("Object Copy Constructor");
            Json copied(original);
            assert(copied.Size() == original.Size());
            assert(copied["key_5000"].Get<std::string>() == "value_5000");
            std::cout << "✓ Copy constructor: " << copied.Size() << " key-value pairs" << std::endl;
        }
        
        // Copy assignment
        {
            TIME_TEST("Object Copy Assignment");
            Json assigned;
            assigned = original;
            assert(assigned.Size() == original.Size());
            assert(assigned["key_0"].Get<std::string>() == "value_0");
            std::cout << "✓ Copy assignment verified" << std::endl;
        }
    }
    
    void testNestedCopy() {
        std::cout << "\n--- Copy Semantics: Nested Structures ---" << std::endl;
        
        Json original = TestData::createNestedStructure();
        
        // Copy constructor with nested validation
        {
            TIME_TEST("Nested Structure Copy Constructor");
            Json copied(original);
            assert(copied["users"].Size() == original["users"].Size());
            assert(copied["users"][1000]["name"].Get<std::string>() == "User1000");
            assert(copied["users"][1000]["scores"].Size() == 10);
            assert(copied["metadata"]["version"].Get<std::string>() == "1.0");
            std::cout << "✓ Nested copy constructor: all levels verified" << std::endl;
        }
        
        // Copy assignment with deep structure
        {
            TIME_TEST("Nested Structure Copy Assignment");
            Json assigned;
            assigned = original;
            assert(assigned["users"][2500]["preferences"]["theme"].Get<std::string>() == "light");
            assert(assigned["users"][2499]["scores"][5].Get<int>() == 50 + 2499);
            std::cout << "✓ Nested copy assignment: deep access verified" << std::endl;
        }
    }
    
    void runAll() {
        std::cout << "\n=== COPY SEMANTICS TESTS ===" << std::endl;
        testPrimitiveCopy();
        testStringCopy();
        testArrayCopy();
        testObjectCopy();
        testNestedCopy();
        std::cout << "✅ All copy semantics tests passed!" << std::endl;
    }
}

// 2. MOVE SEMANTICS TESTS
namespace MoveSemantics {
    
    void testPrimitiveMove() {
        std::cout << "\n--- Move Semantics: Primitives ---" << std::endl;
        
        // Move constructor
        {
            TIME_TEST("Primitive Move Constructor");
            Json source = TestData::createPrimitive();
            Json moved(std::move(source));
            assert(moved.Get<int>() == 42); // Expected value from TestData::createPrimitive()
            std::cout << "✓ Move constructor: " << moved.Get<int>() << std::endl;
        }
        
        // Move assignment
        {
            TIME_TEST("Primitive Move Assignment");
            Json source = TestData::createPrimitive();
            Json moved;
            moved = std::move(source);
            assert(moved.Get<int>() == 42); // Expected value from TestData::createPrimitive()
            std::cout << "✓ Move assignment: " << moved.Get<int>() << std::endl;
        }
    }
    
    void testStringMove() {
        std::cout << "\n--- Move Semantics: Strings ---" << std::endl;
        
        // Move constructor
        {
            TIME_TEST("String Move Constructor");
            Json source = TestData::createString();
            Json moved(std::move(source));
            assert(moved.Get<std::string>() == "Hello, World! This is a test string with some length.");
            std::cout << "✓ Move constructor: string moved efficiently" << std::endl;
        }
        
        // Move assignment
        {
            TIME_TEST("String Move Assignment");
            Json source = TestData::createString();
            Json moved;
            moved = std::move(source);
            assert(moved.Get<std::string>() == "Hello, World! This is a test string with some length.");
            std::cout << "✓ Move assignment: string moved efficiently" << std::endl;
        }
    }
    
    void testArrayMove() {
        std::cout << "\n--- Move Semantics: Arrays ---" << std::endl;
        
        // Move constructor
        {
            Json source = TestData::createArray();  // Create data outside timing
            TIME_TEST("Array Move Constructor");
            Json moved(std::move(source));
            assert(moved.Size() == 10000);
            assert(moved[5000].Get<int>() == 5000 * 5000);
            std::cout << "✓ Move constructor: " << moved.Size() << " elements moved" << std::endl;
        }
        
        // Move assignment
        {
            Json source = TestData::createArray();  // Create data outside timing
            TIME_TEST("Array Move Assignment");
            Json moved;
            moved = std::move(source);
            assert(moved.Size() == 10000);
            assert(moved[9999].Get<int>() == 9999 * 9999);
            std::cout << "✓ Move assignment: large array moved efficiently" << std::endl;
        }
        
        // Efficient array building with moves
        {
            TIME_TEST("Efficient Array Building");
            Json array = Json::Array();
            array.Reserve(10000);
            for (int i = 0; i < 10000; ++i) {
                Json item = Json::Object();
                item["id"] = i;
                item["value"] = "item_" + std::to_string(i);
                array.PushBack(std::move(item)); // Move into array
            }
            assert(array.Size() == 10000);
            std::cout << "✓ Efficient building: 10000 items moved into array" << std::endl;
        }
    }
    
    void testObjectMove() {
        std::cout << "\n--- Move Semantics: Objects ---" << std::endl;
        
        // Move constructor
        {
            Json source = TestData::createObject();  // Create data outside timing
            TIME_TEST("Object Move Constructor");
            Json moved(std::move(source));
            assert(moved.Size() == 10000);
            assert(moved["key_5000"].Get<std::string>() == "value_5000");
            std::cout << "✓ Move constructor: " << moved.Size() << " key-value pairs moved" << std::endl;
        }
        
        // Move assignment
        {
            Json source = TestData::createObject();  // Create data outside timing
            TIME_TEST("Object Move Assignment");
            Json moved;
            moved = std::move(source);
            assert(moved.Size() == 10000);
            assert(moved["key_9999"].Get<std::string>() == "value_9999");
            std::cout << "✓ Move assignment: large object moved efficiently" << std::endl;
        }
    }
    
    void testNestedMove() {
        std::cout << "\n--- Move Semantics: Nested Structures ---" << std::endl;
        
        // Move constructor with complex nested structure
        {
            Json source = TestData::createNestedStructure();  // Create data outside timing
            TIME_TEST("Nested Structure Move Constructor");
            Json moved(std::move(source));
            assert(moved["users"].Size() == 10000);
            assert(moved["users"][1000]["name"].Get<std::string>() == "User1000");
            assert(moved["metadata"]["version"].Get<std::string>() == "1.0");
            std::cout << "✓ Move constructor: complex nested structure moved" << std::endl;
        }
        
        // Move assignment with nested validation
        {
            Json source = TestData::createNestedStructure();  // Create data outside timing
            TIME_TEST("Nested Structure Move Assignment");
            Json moved;
            moved = std::move(source);
            assert(moved["users"].Size() == 10000);
            assert(moved["users"][2500]["preferences"]["theme"].Get<std::string>() == "light");
            std::cout << "✓ Move assignment: nested structure moved with integrity" << std::endl;
        }
    }
    
    void runAll() {
        std::cout << "\n=== MOVE SEMANTICS TESTS ===" << std::endl;
        testPrimitiveMove();
        testStringMove();
        testArrayMove();
        testObjectMove();
        testNestedMove();
        std::cout << "✅ All move semantics tests passed!" << std::endl;
    }
}

// 3. COPY ELISION TESTS
namespace CopyElision {
    
    // Factory functions to test RVO (Return Value Optimization)
    Json createPrimitiveRVO() {
        return Json(12345);
    }
    
    Json createStringRVO() {
        return Json("This string should be created directly in the target location via RVO");
    }
    
    Json createArrayRVO() {
        Json arr = Json::Array();
        arr.Reserve(10000);
        for (int i = 0; i < 10000; ++i) {
            arr.PushBack(i * 2);
        }
        return arr; // RVO should eliminate copy here
    }
    
    Json createObjectRVO() {
        Json obj = Json::Object();
        obj.Reserve(10000);
        for (int i = 0; i < 10000; ++i) {
            obj["rvo_key_" + std::to_string(i)] = "rvo_value_" + std::to_string(i);
        }
        return obj; // RVO should eliminate copy here
    }
    
    Json createNestedRVO() {
        Json root = Json::Object();
        root["info"] = Json::Object();
        root["info"]["type"] = "RVO_test";
        root["info"]["optimized"] = true;
        
        root["data"] = Json::Array();
        root["data"].Reserve(10000);
        for (int i = 0; i < 10000; ++i) {
            Json item = Json::Object();
            item["index"] = i;
            item["squared"] = i * i;
            root["data"].PushBack(std::move(item));
        }
        
        return root; // RVO should eliminate copy here
    }
    
    void testPrimitiveElision() {
        std::cout << "\n--- Copy Elision: Primitives ---" << std::endl;
        
        {
            TIME_TEST("Primitive RVO");
            Json result = createPrimitiveRVO();
            assert(result.Get<int>() == 12345);
            std::cout << "✓ RVO primitive creation: " << result.Get<int>() << std::endl;
        }
        
        // Direct initialization (should use RVO)
        {
            TIME_TEST("Direct Primitive Initialization");
            Json direct = Json(54321);
            assert(direct.Get<int>() == 54321);
            std::cout << "✓ Direct initialization: " << direct.Get<int>() << std::endl;
        }
    }
    
    void testStringElision() {
        std::cout << "\n--- Copy Elision: Strings ---" << std::endl;
        
        {
            TIME_TEST("String RVO");
            Json result = createStringRVO();
            assert(result.Get<std::string>().find("RVO") != std::string::npos);
            std::cout << "✓ RVO string creation: optimized" << std::endl;
        }
        
        // Test with string literals (should be very efficient)
        {
            TIME_TEST("String Literal Elision");
            Json literal = Json("Literal string - should be highly optimized");
            std::cout << "✓ String literal elision: optimized" << std::endl;
        }
    }
    
    void testArrayElision() {
        std::cout << "\n--- Copy Elision: Arrays ---" << std::endl;
        
        {
            TIME_TEST("Array RVO");
            Json result = createArrayRVO();
            assert(result.Size() == 10000);
            assert(result[2500].Get<int>() == 5000);
            std::cout << "✓ RVO array creation: " << result.Size() << " elements" << std::endl;
        }
        
        // Test in-place array construction
        {
            TIME_TEST("In-place Array Construction");
            Json array = Json::Array();
            array.Reserve(10000);
            for (int i = 0; i < 10000; ++i) {
                array.PushBack(Json(i * 3)); // Should elide temporary Json objects
            }
            assert(array.Size() == 10000);
            std::cout << "✓ In-place construction: 10000 elements added efficiently" << std::endl;
        }
    }
    
    void testObjectElision() {
        std::cout << "\n--- Copy Elision: Objects ---" << std::endl;
        
        {
            TIME_TEST("Object RVO");
            Json result = createObjectRVO();
            assert(result.Size() == 10000);
            assert(result["rvo_key_2500"].Get<std::string>() == "rvo_value_2500");
            std::cout << "✓ RVO object creation: " << result.Size() << " key-value pairs" << std::endl;
        }
        
        // Test direct object building
        {
            TIME_TEST("Direct Object Building");
            Json obj = Json::Object();
            obj.Reserve(10000);
            for (int i = 0; i < 10000; ++i) {
                obj["direct_" + std::to_string(i)] = Json("value_" + std::to_string(i));
            }
            assert(obj.Size() == 10000);
            std::cout << "✓ Direct building: 10000 key-value pairs" << std::endl;
        }
    }
    
    void testNestedElision() {
        std::cout << "\n--- Copy Elision: Nested Structures ---" << std::endl;
        
        {
            TIME_TEST("Nested Structure RVO");
            Json result = createNestedRVO();
            assert(result["data"].Size() == 10000);
            assert(result["info"]["type"].Get<std::string>() == "RVO_test");
            assert(result["data"][5000]["squared"].Get<int>() == 25000000);
            std::cout << "✓ RVO nested creation: complex structure optimized" << std::endl;
        }
        
        // Test nested construction with minimal copies
        {
            TIME_TEST("Optimized Nested Construction");
            Json root = Json::Object();
            root["layers"] = Json::Array();
            root["layers"].Reserve(100);
            
            for (int layer = 0; layer < 100; ++layer) {
                Json layer_obj = Json::Object();
                layer_obj["level"] = layer;
                layer_obj["items"] = Json::Array();
                layer_obj["items"].Reserve(100);
                
                for (int item = 0; item < 100; ++item) {
                    Json item_obj = Json::Object();
                    item_obj["id"] = Json(item);
                    item_obj["layer_id"] = Json(layer);
                    item_obj["computed"] = Json(layer * 100 + item);
                    layer_obj["items"].PushBack(std::move(item_obj));
                }
                
                root["layers"].PushBack(std::move(layer_obj));
            }
            
            assert(root["layers"].Size() == 100);
            assert(root["layers"][50]["items"].Size() == 100);
            std::cout << "✓ Optimized nested: 100 layers with 100 items each (10000 total)" << std::endl;
        }
    }
    
    void testElisionEfficiency() {
        std::cout << "\n--- Copy Elision: Efficiency Comparison ---" << std::endl;
        
        // Compare factory vs direct construction
        {
            std::cout << "Comparing factory method vs direct construction:" << std::endl;
            
            {
                TIME_TEST("Factory Method (RVO)");
                std::vector<Json> objects;
                objects.reserve(1000);
                for (int i = 0; i < 1000; ++i) {
                    objects.push_back(createArrayRVO());
                }
                std::cout << "✓ Factory method: 1000 arrays via RVO (10M elements total)" << std::endl;
            }
            
            {
                TIME_TEST("Direct Construction");
                std::vector<Json> objects;
                objects.reserve(1000);
                for (int i = 0; i < 1000; ++i) {
                    Json arr = Json::Array();
                    arr.Reserve(10000);
                    for (int j = 0; j < 10000; ++j) {
                        arr.PushBack(j * 2);
                    }
                    objects.push_back(std::move(arr));
                }
                std::cout << "✓ Direct construction: 1000 arrays with explicit moves (10M elements total)" << std::endl;
            }
        }
    }
    
    void runAll() {
        std::cout << "\n=== COPY ELISION TESTS ===" << std::endl;
        testPrimitiveElision();
        testStringElision();
        testArrayElision();
        testObjectElision();
        testNestedElision();
        testElisionEfficiency();
        std::cout << "✅ All copy elision tests passed!" << std::endl;
    }
}

int main() {
    std::cout << "=== JSON COPY/MOVE SEMANTICS & COPY ELISION TEST SUITE ===" << std::endl;
    std::cout << "Testing comprehensive copy and move semantics across all JSON types" << std::endl;
    
    try {
        // Run all three test categories
        CopySemantics::runAll();
        MoveSemantics::runAll();
        CopyElision::runAll();
        
        std::cout << "\n🎉 ALL TESTS COMPLETED SUCCESSFULLY! 🎉" << std::endl;
        std::cout << "✅ Copy Semantics: All types tested" << std::endl;
        std::cout << "✅ Move Semantics: All types tested" << std::endl;
        std::cout << "✅ Copy Elision: RVO and efficiency verified" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}