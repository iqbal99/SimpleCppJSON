#ifndef JSON_IMPL_H
#define JSON_IMPL_H

#include "Json.h"
#include <variant>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <unordered_set>
#include <atomic>

class Json::Impl {
public:
    using Number = double;
    using Array = std::vector<Json>;
    
    // SMART CONTAINER SELECTION: Optimized unordered_map with intelligent sizing
    class SmartObject : public std::unordered_map<std::string, Json> {
    private:
        mutable size_t access_count_ = 0;
        static constexpr size_t SMALL_OBJECT_THRESHOLD = 8;
        static constexpr size_t MEDIUM_OBJECT_THRESHOLD = 32;
        
    public:
        SmartObject() {
            // Start with small capacity optimized for typical JSON objects
            reserve(SMALL_OBJECT_THRESHOLD);
        }
        
                 // Override operator[] to add smart growth
         Json& operator[](const std::string& key) {
             access_count_++;
             
             // Smart capacity management based on usage patterns
             // Check if we need to reserve more space based on load factor
             if (load_factor() > 0.75) {  // Load factor approaching limit
                 size_t new_bucket_count;
                 if (size() < SMALL_OBJECT_THRESHOLD) {
                     new_bucket_count = SMALL_OBJECT_THRESHOLD;
                 } else if (size() < MEDIUM_OBJECT_THRESHOLD) {
                     new_bucket_count = MEDIUM_OBJECT_THRESHOLD;
                 } else {
                     new_bucket_count = bucket_count() * 2;  // Exponential growth for large objects
                 }
                 
                 if (new_bucket_count > bucket_count()) {
                     reserve(new_bucket_count);
                 }
             }
             
             return std::unordered_map<std::string, Json>::operator[](key);
         }
        
        // Override at() to track access patterns
        const Json& at(const std::string& key) const {
            access_count_++;
            return std::unordered_map<std::string, Json>::at(key);
        }
        
        // Override contains to track access patterns  
        bool contains(const std::string& key) const {
            access_count_++;
            return std::unordered_map<std::string, Json>::contains(key);
        }
        
        // Smart reserve that considers object size patterns
        void smart_reserve(size_t capacity) {
            if (capacity <= SMALL_OBJECT_THRESHOLD) {
                reserve(SMALL_OBJECT_THRESHOLD);
            } else if (capacity <= MEDIUM_OBJECT_THRESHOLD) {
                reserve(MEDIUM_OBJECT_THRESHOLD);
            } else {
                // For large objects, reserve with some headroom
                reserve(capacity + capacity / 4);
            }
        }
        
        // Access pattern analysis
        size_t get_access_count() const { return access_count_; }
        void reset_access_count() { access_count_ = 0; }
    };
    
    using Object = SmartObject;  // Use smart object selection
    
    // Copy-on-Write data structure
    struct COW_Data {
        std::variant<std::nullptr_t, bool, Number, std::string, Array, Object> value_;
        
        COW_Data() : value_(nullptr) {}
        COW_Data(std::variant<std::nullptr_t, bool, Number, std::string, Array, Object>&& val) 
            : value_(std::move(val)) {}
        
        // Non-copyable, non-movable to ensure proper COW semantics
        COW_Data(const COW_Data&) = delete;
        COW_Data& operator=(const COW_Data&) = delete;
        COW_Data(COW_Data&&) = delete;
        COW_Data& operator=(COW_Data&&) = delete;
    };

    // COW implementation
    mutable std::shared_ptr<COW_Data> data_;

    void EnsureUnique() const {
        if (data_ && data_.use_count() > 1) {
            // Create a deep copy
            auto new_data = std::make_shared<COW_Data>();
            new_data->value_ = data_->value_;  // This will deep copy the variant
            data_ = std::move(new_data);
        }
    }

    // String interning for frequently used keys
    static thread_local std::unordered_set<std::string> string_pool_;
    static const std::string& InternString(const std::string& str);

    // OPTIMIZED Memory pool for Json::Impl objects with O(1) operations
    static thread_local std::vector<std::unique_ptr<Impl>> object_pool_;
    static thread_local size_t pool_index_;
    static std::unique_ptr<Impl> AcquireImpl();
    static void ReleaseImpl(std::unique_ptr<Impl> impl);

    explicit Impl(std::variant<std::nullptr_t, bool, Number, std::string, Array, Object>&& value = nullptr) noexcept 
        : data_(std::make_shared<COW_Data>(std::move(value))) {}
    
    Impl(const Impl& other) : data_(other.data_) {}  // Shallow copy for COW
    
    Impl(Impl&& other) noexcept : data_(std::move(other.data_)) {}
    
    Impl& operator=(const Impl& other) {
        if (this != &other) {
            data_ = other.data_;  // Shallow copy for COW
        }
        return *this;
    }
    
    Impl& operator=(Impl&& other) noexcept {
        if (this != &other) {
            data_ = std::move(other.data_);
        }
        return *this;
    }
    
    ~Impl() = default;

    // Value access
    [[nodiscard]] Type GetType() const noexcept;
    [[nodiscard]] bool GetBoolean() const;
    [[nodiscard]] Number GetNumber() const;
    [[nodiscard]] const std::string& GetString() const;
    [[nodiscard]] const Array& GetArray() const;
    [[nodiscard]] const Object& GetObject() const;
    [[nodiscard]] Array& GetArray();
    [[nodiscard]] Object& GetObject();

    // Value modification
    void SetNull() noexcept;
    void SetBoolean(bool value) noexcept;
    void SetNumber(Number value) noexcept;
    void SetString(std::string value);
    void SetArray();
    void SetObject();

    // Array operations
    Json& At(size_t index);
    const Json& At(size_t index) const;
    void PushBack(Json value);
    void PopBack();
    void ReserveArray(size_t capacity);
    [[nodiscard]] size_t Size() const noexcept;

    // Object operations
    Json& operator[](std::string_view key);
    const Json& At(std::string_view key) const;
    [[nodiscard]] bool Contains(std::string_view key) const noexcept;
    void Remove(std::string_view key);
    void ReserveObject(size_t capacity);
    [[nodiscard]] std::vector<std::string> Keys() const;

    // Serialization
    [[nodiscard]] std::string ToString(bool pretty) const;

private:
    template<typename T>
    [[nodiscard]] const T& Get() const {
        return std::get<T>(data_->value_);
    }

    template<typename T>
    [[nodiscard]] T& Get() {
        return std::get<T>(data_->value_);
    }
};

#endif // JSON_IMPL_H
