#ifndef JSON_H
#define JSON_H

#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <string_view>
#include <concepts>
#include <stdexcept>
#include <iterator>
#include <tuple>

// Forward declarations
namespace detail {
    class JsonImpl;
}

// Forward declare implementation class
namespace detail {
    class JsonImpl;
}

// Forward declare exception class
class JsonException;

class Json {
private:
    // Forward declarations of iterator classes
    class IteratorImpl;
    class ConstIteratorImpl;
    class ObjectIteratorImpl;
    class ConstObjectIteratorImpl;

public:
    // Value types that can be stored in Json
    enum class Type {
        Null,
        Boolean,
        Number,
        String,
        Array,
        Object
    };

    // Constructors
    Json() noexcept;  // Creates null
    Json(std::nullptr_t) noexcept;
    Json(bool value) noexcept;
    Json(int value) noexcept;
    Json(int64_t value) noexcept;
    Json(double value) noexcept;
    Json(const char* value);
    Json(std::string_view value);
    Json(std::string value);

    // Copy operations
    Json(const Json& other);
    Json& operator=(const Json& other);

    // Move operations
    Json(Json&& other) noexcept;
    Json& operator=(Json&& other) noexcept;

    // Destructor
    ~Json();

    // Factory methods
    [[nodiscard]] static Json Array();
    [[nodiscard]] static Json Object();
    [[nodiscard]] static Json Parse(std::string_view json_string);

    // Type checking
    [[nodiscard]] bool IsNull() const noexcept;
    [[nodiscard]] bool IsBoolean() const noexcept;
    [[nodiscard]] bool IsNumber() const noexcept;
    [[nodiscard]] bool IsString() const noexcept;
    [[nodiscard]] bool IsArray() const noexcept;
    [[nodiscard]] bool IsObject() const noexcept;
    [[nodiscard]] Type GetType() const noexcept;

    // Generic value access
    template<typename T>
    requires std::same_as<T, bool> ||
             std::integral<T> ||
             std::floating_point<T> ||
             std::convertible_to<T, std::string_view>
    [[nodiscard]] T Get() const;

    template<typename T>
    requires std::same_as<T, bool> ||
             std::integral<T> ||
             std::floating_point<T> ||
             std::convertible_to<T, std::string_view>
    void Set(T value);

    // Safe access
    template<typename T>
    [[nodiscard]] std::optional<T> TryGet() const noexcept;

    // Array operations
    Json& operator[](size_t index);
    const Json& operator[](size_t index) const;
    void PushBack(Json value);
    void PopBack();
    void Reserve(size_t capacity);  // Reserve capacity for arrays/objects
    [[nodiscard]] size_t Size() const;

    // Object operations
    Json& operator[](std::string_view key);
    const Json& operator[](std::string_view key) const;
    [[nodiscard]] bool Contains(std::string_view key) const;
    void Remove(std::string_view key);
    [[nodiscard]] std::vector<std::string> Keys() const;

    // Serialization
    [[nodiscard]] std::string ToString(bool pretty = false) const;

    // Public iterator types that wrap the implementation
    class Iterator;
    class ConstIterator;
    class ObjectIterator;
    class ConstObjectIterator;

    struct KeyValue {
        std::string key;  // Changed from string_view to string to avoid dangling references
        Json* value_ptr;  // Use pointer instead of reference to allow assignment
        
        KeyValue(std::string_view k, Json& v) : key(k), value_ptr(&v) {}
        
        Json& value() { return *value_ptr; }
        const Json& value() const { return *value_ptr; }
        
        // Structured binding support
        template<size_t N>
        decltype(auto) get() const {
            if constexpr (N == 0) return key;
            else if constexpr (N == 1) return value();
        }
    };

    struct ConstKeyValue {
        std::string key;  // Changed from string_view to string to avoid dangling references
        const Json* value_ptr;  // Use pointer instead of reference to allow assignment
        
        ConstKeyValue(std::string_view k, const Json& v) : key(k), value_ptr(&v) {}
        
        const Json& value() const { return *value_ptr; }
        
        // Structured binding support
        template<size_t N>
        decltype(auto) get() const {
            if constexpr (N == 0) return key;
            else if constexpr (N == 1) return value();
        }
    };

    // Array iteration
    [[nodiscard]] Iterator begin() noexcept;
    [[nodiscard]] Iterator end() noexcept;
    [[nodiscard]] ConstIterator begin() const noexcept;
    [[nodiscard]] ConstIterator end() const noexcept;
    [[nodiscard]] ConstIterator cbegin() const noexcept;
    [[nodiscard]] ConstIterator cend() const noexcept;

    // Object iteration
    [[nodiscard]] ObjectIterator object_begin() noexcept;
    [[nodiscard]] ObjectIterator object_end() noexcept;
    [[nodiscard]] ConstObjectIterator object_begin() const noexcept;
    [[nodiscard]] ConstObjectIterator object_end() const noexcept;
    [[nodiscard]] ConstObjectIterator object_cbegin() const noexcept;
    [[nodiscard]] ConstObjectIterator object_cend() const noexcept;

    // Range helper classes
    class ObjectRange {
    public:
        explicit ObjectRange(Json& j) noexcept : json_(j) {}
        [[nodiscard]] ObjectIterator begin() noexcept;
        [[nodiscard]] ObjectIterator end() noexcept;
    private:
        Json& json_;
    };

    class ConstObjectRange {
    public:
        explicit ConstObjectRange(const Json& j) noexcept : json_(j) {}
        [[nodiscard]] ConstObjectIterator begin() const noexcept;
        [[nodiscard]] ConstObjectIterator end() const noexcept;
    private:
        const Json& json_;
    };

    [[nodiscard]] ObjectRange ObjectItems() noexcept {
        return ObjectRange(*this);
    }

    [[nodiscard]] ConstObjectRange ObjectItems() const noexcept {
        return ConstObjectRange(*this);
    }

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    
    // Validity check helpers for moved-from object access
    void ensure_valid() const; // Implementation in Json.cpp
    
    bool is_valid() const noexcept { 
        return impl_ != nullptr; 
    }
};

// Exception classes
class JsonException : public std::runtime_error {
public:
    explicit JsonException(const std::string& message);
    explicit JsonException(const char* message);
};

class JsonTypeError : public JsonException {
public:
    JsonTypeError(Json::Type expected, Json::Type actual);
};

class JsonParseError : public JsonException {
public:
    JsonParseError(const std::string& message, size_t line = 0, size_t column = 0);
    [[nodiscard]] size_t Line() const noexcept;
    [[nodiscard]] size_t Column() const noexcept;
private:
    size_t line_;
    size_t column_;
};

// Array Iterator
class Json::Iterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = Json;
    using difference_type = std::ptrdiff_t;
    using pointer = Json*;
    using reference = Json&;

    Iterator() noexcept;
    Iterator& operator++();
    Iterator operator++(int);
    Iterator& operator--();
    Iterator operator--(int);
    reference operator*() const;
    pointer operator->() const;
    bool operator==(const Iterator& other) const noexcept;
    bool operator!=(const Iterator& other) const noexcept;

private:
    friend class Json;
    explicit Iterator(Impl* impl, size_t index);
    Impl* impl_;
    size_t index_;
};

// Const Array Iterator
class Json::ConstIterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = const Json;
    using difference_type = std::ptrdiff_t;
    using pointer = const Json*;
    using reference = const Json&;

    ConstIterator() noexcept;
    ConstIterator(const Iterator& it) noexcept;
    ConstIterator& operator++();
    ConstIterator operator++(int);
    ConstIterator& operator--();
    ConstIterator operator--(int);
    reference operator*() const;
    pointer operator->() const;
    bool operator==(const ConstIterator& other) const noexcept;
    bool operator!=(const ConstIterator& other) const noexcept;

private:
    friend class Json;
    explicit ConstIterator(const Impl* impl, size_t index);
    const Impl* impl_;
    size_t index_;
};

// Object Iterator
class Json::ObjectIterator {
public:
    using iterator_category = std::forward_iterator_tag;  // Changed from bidirectional to forward
    using value_type = KeyValue;
    using difference_type = std::ptrdiff_t;
    using pointer = KeyValue*;
    using reference = KeyValue;

    ObjectIterator() noexcept;
    ObjectIterator(const ObjectIterator& other);  // Add copy constructor
    ObjectIterator(ObjectIterator&& other) noexcept;  // Add move constructor
    ObjectIterator& operator=(const ObjectIterator& other);  // Add copy assignment
    ObjectIterator& operator=(ObjectIterator&& other) noexcept;  // Add move assignment
    ~ObjectIterator();  // Add destructor
    
    ObjectIterator& operator++();
    ObjectIterator operator++(int);
    // Removed operator-- methods since unordered_map doesn't support bidirectional iteration
    reference operator*();
    KeyValue* operator->();
    bool operator==(const ObjectIterator& other) const noexcept;
    bool operator!=(const ObjectIterator& other) const noexcept;

private:
    friend class Json;
    friend class Json::ObjectRange;
    class ObjectIterImpl;
    std::unique_ptr<ObjectIterImpl> impl_;
    
    explicit ObjectIterator(Impl* json_impl);
    ObjectIterator(Impl* json_impl, bool end);
};

// Const Object Iterator
class Json::ConstObjectIterator {
public:
    using iterator_category = std::forward_iterator_tag;  // Changed from bidirectional to forward
    using value_type = ConstKeyValue;
    using difference_type = std::ptrdiff_t;
    using pointer = ConstKeyValue*;
    using reference = ConstKeyValue;

    ConstObjectIterator() noexcept;
    ConstObjectIterator(const ConstObjectIterator& other);  // Add copy constructor
    ConstObjectIterator(ConstObjectIterator&& other) noexcept;  // Add move constructor
    ConstObjectIterator& operator=(const ConstObjectIterator& other);  // Add copy assignment
    ConstObjectIterator& operator=(ConstObjectIterator&& other) noexcept;  // Add move assignment
    ~ConstObjectIterator();  // Add destructor
    
    ConstObjectIterator(const ObjectIterator& it) noexcept;
    ConstObjectIterator& operator++();
    ConstObjectIterator operator++(int);
    // Removed operator-- methods since unordered_map doesn't support bidirectional iteration
    reference operator*();
    ConstKeyValue* operator->();
    bool operator==(const ConstObjectIterator& other) const noexcept;
    bool operator!=(const ConstObjectIterator& other) const noexcept;

private:
    friend class Json;
    friend class Json::ConstObjectRange;
    class ConstObjectIterImpl;
    std::unique_ptr<ConstObjectIterImpl> impl_;
    
    explicit ConstObjectIterator(const Impl* json_impl);
    ConstObjectIterator(const Impl* json_impl, bool end);
};

// Structured binding support
namespace std {
    template<>
    struct tuple_size<Json::KeyValue> : integral_constant<size_t, 2> {};
    
    template<>
    struct tuple_element<0, Json::KeyValue> {
        using type = string;
    };
    
    template<>
    struct tuple_element<1, Json::KeyValue> {
        using type = Json&;
    };
    
    template<>
    struct tuple_size<Json::ConstKeyValue> : integral_constant<size_t, 2> {};
    
    template<>
    struct tuple_element<0, Json::ConstKeyValue> {
        using type = string;
    };
    
    template<>
    struct tuple_element<1, Json::ConstKeyValue> {
        using type = const Json&;
    };
}

// Note: Template implementations are now in JsonTemplates.h
// Include JsonImpl.h before JsonTemplates.h in source files that need templates

#endif // JSON_H
