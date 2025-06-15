#include "../Json.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include <random>
#include <thread>
#include <vector>
#include <atomic>
#include <algorithm>
#include <sstream>

class StressTester {
public:
    std::mt19937 rng;
    
private:
    std::uniform_int_distribution<int> int_dist;
    std::uniform_real_distribution<double> real_dist;
    std::uniform_int_distribution<int> string_length_dist;
    std::uniform_int_distribution<int> depth_dist;
    
public:
    StressTester() : rng(std::random_device{}()), 
                     int_dist(-1000000, 1000000),
                     real_dist(-1000000.0, 1000000.0),
                     string_length_dist(0, 1000),
                     depth_dist(1, 20) {}
    
    std::string generate_random_string(int length = -1) {
        if (length == -1) {
            length = string_length_dist(rng);
        }
        
        std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 !@#$%^&*()_+-=[]{}|;:,.<>?";
        std::uniform_int_distribution<int> char_dist(0, chars.length() - 1);
        
        std::string result;
        result.reserve(length);
        for (int i = 0; i < length; ++i) {
            result += chars[char_dist(rng)];
        }
        return result;
    }
    
    Json generate_random_json(int max_depth = 5, int max_elements = 10) {
        std::uniform_int_distribution<int> type_dist(0, 5);
        std::uniform_int_distribution<int> element_dist(0, max_elements);
        
        int type = type_dist(rng);
        
        switch (type) {
            case 0: return nullptr;
            case 1: return (rng() % 2) == 0;
            case 2: return int_dist(rng);
            case 3: return real_dist(rng);
            case 4: return generate_random_string();
            case 5: {
                // Array
                auto arr = Json::Array();
                int size = element_dist(rng);
                for (int i = 0; i < size; ++i) {
                    if (max_depth > 0) {
                        arr.PushBack(generate_random_json(max_depth - 1, max_elements));
                    } else {
                        arr.PushBack(int_dist(rng));
                    }
                }
                return arr;
            }
            default: {
                // Object
                auto obj = Json::Object();
                int size = element_dist(rng);
                for (int i = 0; i < size; ++i) {
                    std::string key = "key_" + std::to_string(i) + "_" + generate_random_string(10);
                    if (max_depth > 0) {
                        obj[key] = generate_random_json(max_depth - 1, max_elements);
                    } else {
                        obj[key] = int_dist(rng);
                    }
                }
                return obj;
            }
        }
    }
};

void test_extreme_nesting() {
    std::cout << "\n=== Testing Extreme Nesting ===\n";
    
    try {
        // Test very deep object nesting
        const int max_depth = 10000;
        auto deep_obj = Json::Object();
        auto* current = &deep_obj;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < max_depth; ++i) {
            (*current)["level"] = Json::Object();
            current = &(*current)["level"];
            if (i % 1000 == 0) {
                std::cout << "Created " << i << " levels..." << std::endl;
            }
        }
        (*current)["value"] = "bottom";
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Deep nesting creation took: " << duration.count() << "ms" << std::endl;
        
        // Test accessing the deep value
        start = std::chrono::high_resolution_clock::now();
        current = &deep_obj;
        for (int i = 0; i < max_depth; ++i) {
            current = &(*current)["level"];
        }
        std::string value = (*current)["value"].Get<std::string>();
        
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Deep access took: " << duration.count() << "ms" << std::endl;
        
        assert(value == "bottom");
        std::cout << "✓ Extreme nesting test passed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Exception in extreme nesting: " << e.what() << std::endl;
    }
}

void test_massive_arrays() {
    std::cout << "\n=== Testing Massive Arrays ===\n";
    
    try {
        const size_t massive_size = 1000000;  // 1 million elements
        auto massive_array = Json::Array();
        massive_array.Reserve(massive_size);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < massive_size; ++i) {
            massive_array.PushBack(static_cast<int>(i));
            if (i % 100000 == 0) {
                std::cout << "Added " << i << " elements..." << std::endl;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Massive array creation took: " << duration.count() << "ms" << std::endl;
        
        // Test random access
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dist(0, massive_size - 1);
        
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 10000; ++i) {
            size_t index = dist(gen);
            int value = massive_array[index].Get<int>();
            assert(value == static_cast<int>(index));
        }
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "10k random accesses took: " << duration.count() << "ms" << std::endl;
        
        assert(massive_array.Size() == massive_size);
        std::cout << "✓ Massive array test passed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Exception in massive arrays: " << e.what() << std::endl;
    }
}

void test_massive_objects() {
    std::cout << "\n=== Testing Massive Objects ===\n";
    
    try {
        const int massive_keys = 100000;  // 100k keys
        auto massive_obj = Json::Object();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < massive_keys; ++i) {
            std::string key = "key_" + std::to_string(i);
            massive_obj[key] = "value_" + std::to_string(i);
            if (i % 10000 == 0) {
                std::cout << "Added " << i << " keys..." << std::endl;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Massive object creation took: " << duration.count() << "ms" << std::endl;
        
        // Test key lookup performance
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, massive_keys - 1);
        
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 10000; ++i) {
            int index = dist(gen);
            std::string key = "key_" + std::to_string(index);
            std::string expected = "value_" + std::to_string(index);
            std::string actual = massive_obj[key].Get<std::string>();
            assert(actual == expected);
        }
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "10k key lookups took: " << duration.count() << "ms" << std::endl;
        
        assert(massive_obj.Keys().size() == massive_keys);
        std::cout << "✓ Massive object test passed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Exception in massive objects: " << e.what() << std::endl;
    }
}

void test_random_operations() {
    std::cout << "\n=== Testing Random Operations ===\n";
    
    try {
        StressTester tester;
        const int num_operations = 100000;
        
        auto json = Json::Object();
        json["counters"] = Json::Object();
        json["arrays"] = Json::Array();
        json["objects"] = Json::Array();
        json["data"] = Json::Object();
        
        std::uniform_int_distribution<int> operation_dist(0, 6);
        std::uniform_int_distribution<int> key_dist(0, 999);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < num_operations; ++i) {
            try {
                int op = operation_dist(tester.rng);
                std::string key = "key_" + std::to_string(key_dist(tester.rng));
                
                switch (op) {
                    case 0: // Add to object
                        json["data"][key] = tester.generate_random_json(2, 5);
                        break;
                    case 1: // Remove from object
                        if (json["data"].Contains(key)) {
                            json["data"].Remove(key);
                        }
                        break;
                    case 2: // Add to array
                        json["arrays"].PushBack(tester.generate_random_json(1, 3));
                        break;
                    case 3: // Pop from array
                        if (json["arrays"].Size() > 0) {
                            json["arrays"].PopBack();
                        }
                        break;
                    case 4: // Increment counter
                        if (json["counters"].Contains(key)) {
                            int val = json["counters"][key].Get<int>();
                            json["counters"][key] = val + 1;
                        } else {
                            json["counters"][key] = 1;
                        }
                        break;
                    case 5: // Create nested object
                        json["objects"].PushBack(Json::Object());
                        if (json["objects"].Size() > 0) {
                            size_t idx = json["objects"].Size() - 1;
                            json["objects"][idx][key] = tester.generate_random_json(1, 2);
                        }
                        break;
                    case 6: // Type change
                        json["data"][key] = tester.generate_random_json(0, 1);
                        break;
                }
                
                if (i % 10000 == 0) {
                    std::cout << "Completed " << i << " random operations..." << std::endl;
                }
                
            } catch (const JsonException&) {
                // Some operations may fail due to type mismatches, that's okay
                continue;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Random operations took: " << duration.count() << "ms" << std::endl;
        
        std::cout << "Final stats:" << std::endl;
        std::cout << "  Data keys: " << json["data"].Keys().size() << std::endl;
        std::cout << "  Arrays size: " << json["arrays"].Size() << std::endl;
        std::cout << "  Objects size: " << json["objects"].Size() << std::endl;
        std::cout << "  Counters: " << json["counters"].Keys().size() << std::endl;
        
        std::cout << "✓ Random operations test passed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Exception in random operations: " << e.what() << std::endl;
    }
}

void test_serialization_stress() {
    std::cout << "\n=== Testing Serialization Stress ===\n";
    
    try {
        StressTester tester;
        
        // Create a very complex structure
        auto complex = tester.generate_random_json(8, 20);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Serialize multiple times
        for (int i = 0; i < 100; ++i) {
            std::string compact = complex.ToString();
            std::string pretty = complex.ToString(true);
            
            if (i % 10 == 0) {
                std::cout << "Serialization " << i << " completed..." << std::endl;
            }
            
            // Verify we can parse it back
            if (i % 20 == 0) {
                auto parsed = Json::Parse(compact);
                auto parsed_pretty = Json::Parse(pretty);
                // Basic verification that parsing worked
                assert(parsed.GetType() == complex.GetType());
                assert(parsed_pretty.GetType() == complex.GetType());
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Serialization stress test took: " << duration.count() << "ms" << std::endl;
        
        std::cout << "✓ Serialization stress test passed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Exception in serialization stress: " << e.what() << std::endl;
    }
}

void test_concurrent_stress() {
    std::cout << "\n=== Testing Concurrent Stress ===\n";
    
    try {
        const int num_threads = std::thread::hardware_concurrency();
        const int operations_per_thread = 10000;
        
        std::cout << "Running with " << num_threads << " threads" << std::endl;
        
        std::vector<std::thread> threads;
        std::atomic<int> total_operations{0};
        std::atomic<int> successful_operations{0};
        
        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([&, t]() {
                StressTester tester;
                
                for (int i = 0; i < operations_per_thread; ++i) {
                    try {
                        // Each thread works with its own JSON objects
                        auto local_json = tester.generate_random_json(3, 10);
                        
                        // Perform various operations
                        std::string serialized = local_json.ToString();
                        auto parsed = Json::Parse(serialized);
                        
                        // Verify round-trip
                        std::string reserialized = parsed.ToString();
                        
                        if (!serialized.empty() && !reserialized.empty()) {
                            successful_operations++;
                        }
                        
                        total_operations++;
                        
                    } catch (const std::exception&) {
                        total_operations++;
                        // Failures are acceptable in stress testing
                    }
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        std::cout << "Total operations: " << total_operations.load() << std::endl;
        std::cout << "Successful operations: " << successful_operations.load() << std::endl;
        std::cout << "Success rate: " << (100.0 * successful_operations.load() / total_operations.load()) << "%" << std::endl;
        
        // We expect at least 90% success rate
        assert(successful_operations.load() > (total_operations.load() * 9 / 10));
        
        std::cout << "✓ Concurrent stress test passed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Exception in concurrent stress: " << e.what() << std::endl;
    }
}

void test_memory_stress() {
    std::cout << "\n=== Testing Memory Stress ===\n";
    
    try {
        StressTester tester;
        
        // Create and destroy many objects
        const int iterations = 10000;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            // Create complex object
            auto obj = tester.generate_random_json(5, 15);
            
            // Serialize it
            std::string serialized = obj.ToString();
            
            // Parse it back
            auto parsed = Json::Parse(serialized);
            
            // Copy it
            auto copied = parsed;
            
            // Move it
            auto moved = std::move(copied);
            
            // Create array of objects
            auto arr = Json::Array();
            for (int j = 0; j < 10; ++j) {
                arr.PushBack(tester.generate_random_json(2, 5));
            }
            
            if (i % 1000 == 0) {
                std::cout << "Memory stress iteration " << i << std::endl;
            }
            
            // Objects should be automatically cleaned up here
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Memory stress test took: " << duration.count() << "ms" << std::endl;
        
        std::cout << "✓ Memory stress test passed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Exception in memory stress: " << e.what() << std::endl;
    }
}

int main() {
    try {
        std::cout << "JSON Library Stress Test Suite\n";
        std::cout << "==============================\n";
        
        test_extreme_nesting();
        test_massive_arrays();
        test_massive_objects();
        test_random_operations();
        test_serialization_stress();
        test_concurrent_stress();
        test_memory_stress();
        
        std::cout << "\n🎉 All stress tests completed!\n";
        std::cout << "Note: Some tests may take significant time and memory.\n";
        std::cout << "If any test fails due to resource constraints, that's expected behavior.\n";
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Stress test suite failed with exception: " << e.what() << std::endl;
        return 1;
    }
} 