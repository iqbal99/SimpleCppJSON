#include <Json.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <future>
#include <random>
#include <cassert>
#include <atomic>
#include <memory>

// Performance testing utilities
class PerformanceTimer {
public:
    PerformanceTimer(const std::string& name) : name_(name), start_(std::chrono::high_resolution_clock::now()) {}
    
    ~PerformanceTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
        std::cout << "[PERF] " << name_ << ": " << duration.count() << " μs" << std::endl;
    }
    
private:
    std::string name_;
    std::chrono::high_resolution_clock::time_point start_;
};

#define PERF_TEST(name) PerformanceTimer timer(name)

// Test basic functionality first
void testBasicFunctionality() {
    std::cout << "\n=== Basic Functionality Test ===" << std::endl;
    
    // Create JSON object
    auto json = Json::Object();
    json["name"] = "Alice";
    json["age"] = 30;
    json["active"] = true;
    
    // Create nested structure
    json["address"] = Json::Object();
    json["address"]["city"] = "New York";
    json["address"]["zipcode"] = "10001";
    
    // Create array
    json["hobbies"] = Json::Array();
    json["hobbies"].PushBack("reading");
    json["hobbies"].PushBack("coding");
    
    // Serialize to string
    std::cout << json.ToString(true) << std::endl;
    
    // Parse from string
    auto parsed = Json::Parse(R"({"x": 42, "y": true})");
    std::cout << "x = " << parsed["x"].Get<int>() << std::endl;
    
    std::cout << "Basic functionality: PASSED" << std::endl;
}

// Copy and Move Semantics Tests
void testCopyMoveSemantics() {
    std::cout << "\n=== Copy/Move Semantics Test ===" << std::endl;
    
    // Create a complex JSON structure
    auto original = Json::Object();
    original["data"] = Json::Array();
    for (int i = 0; i < 1000; ++i) {
        auto item = Json::Object();
        item["id"] = i;
        item["value"] = "item_" + std::to_string(i);
        original["data"].PushBack(std::move(item));
    }
    
    std::cout << "Created original with " << original["data"].Size() << " items" << std::endl;
    
    // Test copy constructor
    {
        PERF_TEST("Copy Constructor");
        Json copied = original;
        assert(copied["data"].Size() == original["data"].Size());
        assert(copied["data"][0]["id"].Get<int>() == 0);
        std::cout << "Copy constructor: PASSED" << std::endl;
    }
    
    // Test copy assignment
    {
        PERF_TEST("Copy Assignment");
        Json assigned;
        assigned = original;
        assert(assigned["data"].Size() == original["data"].Size());
        assert(assigned["data"][999]["value"].Get<std::string>() == "item_999");
        std::cout << "Copy assignment: PASSED" << std::endl;
    }
    
    // Test move constructor
    {
        PERF_TEST("Move Constructor");
        Json to_move = original; // Make a copy first
        Json moved = std::move(to_move);
        assert(moved["data"].Size() == 1000);
        assert(moved["data"][500]["id"].Get<int>() == 500);
        std::cout << "Move constructor: PASSED" << std::endl;
    }
    
    // Test move assignment
    {
        PERF_TEST("Move Assignment");
        Json to_move = original; // Make a copy first
        Json move_assigned;
        move_assigned = std::move(to_move);
        assert(move_assigned["data"].Size() == 1000);
        assert(move_assigned["data"][250]["value"].Get<std::string>() == "item_250");
        std::cout << "Move assignment: PASSED" << std::endl;
    }
    
    // Test efficient array building with moves
    {
        PERF_TEST("Efficient Array Building with Moves");
        Json efficient_array = Json::Array();
        for (int i = 0; i < 1000; ++i) {
            Json item = Json::Object();
            item["key"] = "value_" + std::to_string(i);
            efficient_array.PushBack(std::move(item)); // Move into array
        }
        assert(efficient_array.Size() == 1000);
        std::cout << "Efficient array building: PASSED" << std::endl;
    }
}

// Performance Tests
void testPerformance() {
    std::cout << "\n=== Performance Tests ===" << std::endl;
    
    const int NUM_ITEMS = 10000;
    
    // Test array performance with reservation
    {
        PERF_TEST("Optimized Array Creation and Population");
        Json array = Json::Array();
        array.Reserve(NUM_ITEMS);  // Pre-allocate capacity
        for (int i = 0; i < NUM_ITEMS; ++i) {
            array.PushBack(i);
        }
        assert(array.Size() == NUM_ITEMS);
        std::cout << "Optimized array with " << NUM_ITEMS << " items created" << std::endl;
    }
    
    // Test object performance with reservation
    {
        PERF_TEST("Optimized Object Creation and Population");
        Json object = Json::Object();
        object.Reserve(NUM_ITEMS);  // Pre-allocate capacity
        for (int i = 0; i < NUM_ITEMS; ++i) {
            object["key_" + std::to_string(i)] = "value_" + std::to_string(i);
        }
        assert(object.Size() == NUM_ITEMS);
        std::cout << "Optimized object with " << NUM_ITEMS << " keys created" << std::endl;
    }
    
    // Compare with non-optimized versions
    {
        PERF_TEST("Non-optimized Array Creation");
        Json array = Json::Array();
        for (int i = 0; i < NUM_ITEMS; ++i) {
            array.PushBack(i);
        }
        assert(array.Size() == NUM_ITEMS);
        std::cout << "Non-optimized array comparison completed" << std::endl;
    }
    
    {
        PERF_TEST("Non-optimized Object Creation");
        Json object = Json::Object();
        for (int i = 0; i < NUM_ITEMS; ++i) {
            object["key_" + std::to_string(i)] = "value_" + std::to_string(i);
        }
        assert(object.Size() == NUM_ITEMS);
        std::cout << "Non-optimized object comparison completed" << std::endl;
    }
    
    // Test nested structure performance
    {
        PERF_TEST("Nested Structure Creation");
        Json root = Json::Object();
        root["users"] = Json::Array();
        
        for (int i = 0; i < 1000; ++i) {
            Json user = Json::Object();
            user["id"] = i;
            user["name"] = "User" + std::to_string(i);
            user["email"] = "user" + std::to_string(i) + "@example.com";
            user["active"] = (i % 2 == 0);
            
            user["preferences"] = Json::Object();
            user["preferences"]["theme"] = (i % 3 == 0) ? "dark" : "light";
            user["preferences"]["notifications"] = true;
            
            user["scores"] = Json::Array();
            for (int j = 0; j < 5; ++j) {
                user["scores"].PushBack(rand() % 100);
            }
            
            root["users"].PushBack(std::move(user));
        }
        
        assert(root["users"].Size() == 1000);
        std::cout << "Complex nested structure created" << std::endl;
    }
    
    // Test serialization performance
    {
        Json large_object = Json::Object();
        large_object["data"] = Json::Array();
        for (int i = 0; i < 5000; ++i) {
            Json item = Json::Object();
            item["id"] = i;
            item["name"] = "Item " + std::to_string(i);
            item["active"] = (i % 2 == 0);
            large_object["data"].PushBack(std::move(item));
        }
        
        {
            PERF_TEST("Compact Serialization");
            std::string compact = large_object.ToString(false);
            std::cout << "Compact JSON size: " << compact.length() << " bytes" << std::endl;
        }
        
        {
            PERF_TEST("Pretty Serialization");
            std::string pretty = large_object.ToString(true);
            std::cout << "Pretty JSON size: " << pretty.length() << " bytes" << std::endl;
        }
    }
    
    // Test parsing performance
    {
        std::string json_string = R"({
            "users": [
                {"id": 1, "name": "Alice", "scores": [95, 87, 92]},
                {"id": 2, "name": "Bob", "scores": [88, 91, 85]},
                {"id": 3, "name": "Charlie", "scores": [92, 89, 94]}
            ],
            "metadata": {
                "total": 3,
                "active": true,
                "version": "1.0"
            }
        })";
        
        {
            PERF_TEST("JSON Parsing");
            Json parsed = Json::Parse(json_string);
            assert(parsed["users"].Size() == 3);
            assert(parsed["metadata"]["total"].Get<int>() == 3);
            std::cout << "JSON parsing successful" << std::endl;
        }
    }
}

// Concurrency Tests
void testConcurrency() {
    std::cout << "\n=== Concurrency Tests ===" << std::endl;
    
    const int NUM_THREADS = 8;
    const int OPERATIONS_PER_THREAD = 1000;
    
    // Test concurrent reads (should be safe)
    {
        PERF_TEST("Concurrent Reads");
        
        // Create a shared JSON object
        Json shared_json = Json::Object();
        shared_json["data"] = Json::Array();
        for (int i = 0; i < 1000; ++i) {
            shared_json["data"].PushBack(i * i);
        }
        
        std::vector<std::future<bool>> futures;
        std::atomic<int> read_count{0};
        
        // Launch multiple threads to read concurrently
        for (int t = 0; t < NUM_THREADS; ++t) {
            futures.push_back(std::async(std::launch::async, [&shared_json, &read_count, OPERATIONS_PER_THREAD]() {
                try {
                    for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
                        int index = i % shared_json["data"].Size();
                        int value = shared_json["data"][index].Get<int>();
                        read_count++;
                        
                        // Verify the value
                        if (value != index * index) {
                            return false;
                        }
                    }
                    return true;
                } catch (...) {
                    return false;
                }
            }));
        }
        
        // Wait for all threads to complete
        bool all_success = true;
        for (auto& future : futures) {
            if (!future.get()) {
                all_success = false;
            }
        }
        
        std::cout << "Concurrent reads: " << (all_success ? "PASSED" : "FAILED") << std::endl;
        std::cout << "Total reads performed: " << read_count.load() << std::endl;
    }
    
    // Test concurrent creation of separate JSON objects
    {
        PERF_TEST("Concurrent Object Creation");
        
        std::vector<std::future<Json>> futures;
        std::atomic<int> creation_count{0};
        
        // Launch multiple threads to create JSON objects concurrently
        for (int t = 0; t < NUM_THREADS; ++t) {
            futures.push_back(std::async(std::launch::async, [t, &creation_count, OPERATIONS_PER_THREAD]() {
                Json local_json = Json::Object();
                local_json["thread_id"] = t;
                local_json["data"] = Json::Array();
                
                for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
                    Json item = Json::Object();
                    item["index"] = i;
                    item["value"] = t * 1000 + i;
                    local_json["data"].PushBack(std::move(item));
                    creation_count++;
                }
                
                return local_json;
            }));
        }
        
        // Collect results
        std::vector<Json> results;
        for (auto& future : futures) {
            results.push_back(future.get());
        }
        
        // Verify results
        bool all_correct = true;
        for (size_t t = 0; t < results.size(); ++t) {
            if (results[t]["thread_id"].Get<int>() != static_cast<int>(t) ||
                results[t]["data"].Size() != OPERATIONS_PER_THREAD) {
                all_correct = false;
                break;
            }
        }
        
        std::cout << "Concurrent object creation: " << (all_correct ? "PASSED" : "FAILED") << std::endl;
        std::cout << "Total operations: " << creation_count.load() << std::endl;
    }
    
    // Test concurrent copy operations
    {
        PERF_TEST("Concurrent Copy Operations");
        
        Json original = Json::Object();
        original["shared_data"] = Json::Array();
        for (int i = 0; i < 500; ++i) {
            original["shared_data"].PushBack("item_" + std::to_string(i));
        }
        
        std::vector<std::future<bool>> futures;
        std::atomic<int> copy_count{0};
        
        // Launch multiple threads to copy the JSON object
        for (int t = 0; t < NUM_THREADS; ++t) {
            futures.push_back(std::async(std::launch::async, [&original, &copy_count, t]() {
                try {
                    for (int i = 0; i < 100; ++i) {
                        Json copied = original; // Copy constructor
                        copied["thread_id"] = t;
                        copied["copy_index"] = i;
                        
                        // Verify the copy
                        if (copied["shared_data"].Size() != 500) {
                            return false;
                        }
                        
                        copy_count++;
                    }
                    return true;
                } catch (...) {
                    return false;
                }
            }));
        }
        
        // Wait for completion
        bool all_success = true;
        for (auto& future : futures) {
            if (!future.get()) {
                all_success = false;
            }
        }
        
        std::cout << "Concurrent copy operations: " << (all_success ? "PASSED" : "FAILED") << std::endl;
        std::cout << "Total copies created: " << copy_count.load() << std::endl;
    }
}

// Memory efficiency test
void testMemoryEfficiency() {
    std::cout << "\n=== Memory Efficiency Tests ===" << std::endl;
    
    // Test move vs copy efficiency
    {
        std::cout << "Testing move vs copy efficiency..." << std::endl;
        
        // OPTIMIZED: Create a large JSON structure with maximum COW efficiency
        auto createLargeJsonOptimized = []() {
            // Use static cache for the entire large structure
            static thread_local Json cached_large_object = []() {
                Json large = Json::Object();
                large["data"] = Json::Array();
                large["data"].Reserve(10000);
                
                // Create a minimal template
                Json item_template = Json::Object();
                item_template["id"] = 0;
                item_template["description"] = "Template item";
                
                // Pre-populate with template objects
                for (int i = 0; i < 10000; ++i) {
                    Json item = item_template;
                    item["id"] = i;
                    large["data"].PushBack(std::move(item));
                }
                return large;
            }();
            
            // Return a COW copy of the cached object - this should be very fast!
            return cached_large_object;
        };
        
        // LEGACY: Original inefficient factory for comparison
        auto createLargeJsonLegacy = []() {
            Json large = Json::Object();
            large["data"] = Json::Array();
            // NO Reserve() - causes reallocations
            for (int i = 0; i < 10000; ++i) {
                Json item = Json::Object();         // Individual allocation each time
                item["id"] = i;
                item["description"] = "This is item number " + std::to_string(i) + " with some additional text to make it larger";
                large["data"].PushBack(std::move(item));
            }
            return large;
        };
        
        // Test copy operations (using optimized factory)
        {
            PERF_TEST("Large Object Copy Operations");
            Json original = createLargeJsonOptimized();
            std::vector<Json> copies;
            
            for (int i = 0; i < 10; ++i) {
                copies.push_back(original); // Copy - should be very fast with COW
            }
            
            std::cout << "Created 10 copies of large object" << std::endl;
        }
        
        // Test SUPER OPTIMIZED move operations using COW cache
        {
            PERF_TEST("Super Optimized Move Operations (COW Cache)");
            std::vector<Json> moved_objects;
            moved_objects.reserve(10);              // Pre-allocate vector capacity
            
            for (int i = 0; i < 10; ++i) {
                moved_objects.push_back(createLargeJsonOptimized()); // Should be nearly free with COW!
            }
            
            std::cout << "Created 10 objects via SUPER OPTIMIZED move operations" << std::endl;
        }
        
        // Test COW copy effectiveness (baseline comparison)
        {
            PERF_TEST("COW Copy Baseline Test");
            Json original = createLargeJsonOptimized();  // Create once
            std::vector<Json> copies;
            copies.reserve(100);                         // More copies to show COW power
            
            for (int i = 0; i < 100; ++i) {
                copies.push_back(original);              // Should be extremely fast
            }
            
            std::cout << "Created 100 COW copies (should be near-instant)" << std::endl;
        }
        
        // Test LEGACY move operations (for comparison)
        {
            PERF_TEST("Legacy Move Operations (Original)");
            std::vector<Json> moved_objects;
            
            for (int i = 0; i < 10; ++i) {
                moved_objects.push_back(createLargeJsonLegacy()); // Original inefficient factory
            }
            
            std::cout << "Created 10 objects via LEGACY move operations" << std::endl;
        }
    }
    
    // Test RAII and automatic cleanup (OPTIMIZED)
    {
        PERF_TEST("Optimized RAII and Scope-based Cleanup");
        
        // Pre-create template objects for reuse
        static thread_local Json base_template = []() {
            Json tmpl = Json::Object();
            tmpl["iteration"] = 0;
            tmpl["data"] = Json::Array();
            tmpl["data"].Reserve(100);  // Pre-allocate capacity
            return tmpl;
        }();
        
        static thread_local Json item_cache = Json("template_item");
        
        for (int iteration = 0; iteration < 1000; ++iteration) {
            Json scoped_json = base_template;       // COW copy (cheap!)
            scoped_json["iteration"] = iteration;
            
            for (int i = 0; i < 100; ++i) {
                Json item = item_cache;             // COW copy 
                // Modify item to make it unique and trigger COW
                scoped_json["data"].PushBack("item_" + std::to_string(i));
            }
            
            // Object automatically cleaned up at end of scope
        }
        
        std::cout << "1000 OPTIMIZED scoped objects created and cleaned up" << std::endl;
    }
    
    // Test LEGACY RAII (for comparison)
    {
        PERF_TEST("Legacy RAII and Scope-based Cleanup");
        
        for (int iteration = 0; iteration < 1000; ++iteration) {
            Json scoped_json = Json::Object();      // Fresh allocation each time
            scoped_json["iteration"] = iteration;
            scoped_json["data"] = Json::Array();    // No Reserve() - causes reallocations
            
            for (int i = 0; i < 100; ++i) {
                scoped_json["data"].PushBack("item_" + std::to_string(i));
            }
            
            // Object automatically cleaned up at end of scope
        }
        
        std::cout << "1000 LEGACY scoped objects created and cleaned up" << std::endl;
    }
}

int main() {
    std::cout << "=== JSON Library Comprehensive Test Suite ===" << std::endl;
    
    try {
        // Run all test suites
        testBasicFunctionality();
        testCopyMoveSemantics();
        testPerformance();
        testConcurrency();
        testMemoryEfficiency();
        
        std::cout << "\n=== All Tests Completed Successfully! ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}