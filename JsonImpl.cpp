#include "JsonImpl.h"
#include <sstream>
#include <iomanip>
#include <cassert>
#include <unordered_set>
#include <algorithm>

// String interning implementation
thread_local std::unordered_set<std::string> Json::Impl::string_pool_;

const std::string& Json::Impl::InternString(const std::string& str) {
    auto [it, inserted] = string_pool_.insert(str);
    return *it;
}

// OPTIMIZED Memory pool implementation with O(1) operations and larger capacity
thread_local std::vector<std::unique_ptr<Json::Impl>> Json::Impl::object_pool_;
thread_local size_t Json::Impl::pool_index_ = 0;

// SMART OBJECT IMPLEMENTATION - Now using inheritance-based approach

std::unique_ptr<Json::Impl> Json::Impl::AcquireImpl() {
    static constexpr size_t MAX_POOL_SIZE = 1000;
    static constexpr size_t WARMUP_SIZE = 50;
    
    // Initialize pool on first use
    if (object_pool_.empty() && pool_index_ == 0) {
        object_pool_.reserve(MAX_POOL_SIZE);
        // Pre-allocate some objects for immediate use
        for (size_t i = 0; i < WARMUP_SIZE; ++i) {
            object_pool_.emplace_back(std::make_unique<Impl>());
        }
        pool_index_ = WARMUP_SIZE;
    }
    
    if (pool_index_ > 0) {
        // O(1) acquisition from pre-allocated pool
        auto impl = std::move(object_pool_[--pool_index_]);
        // Reset to null state efficiently
        impl->data_ = std::make_shared<COW_Data>();
        return impl;
    }
    
    // Pool empty, create new object
    return std::make_unique<Impl>();
}

void Json::Impl::ReleaseImpl(std::unique_ptr<Impl> impl) {
    static constexpr size_t MAX_POOL_SIZE = 1000;
    
    if (pool_index_ < MAX_POOL_SIZE) {
        // O(1) insertion into pool using index-based approach
        if (object_pool_.size() <= pool_index_) {
            object_pool_.resize(pool_index_ + 1);
        }
        object_pool_[pool_index_++] = std::move(impl);
    }
    // If pool is full, let the unique_ptr destroy the object naturally
}

Json::Type Json::Impl::GetType() const noexcept {
    return static_cast<Type>(data_->value_.index());
}

bool Json::Impl::GetBoolean() const {
    try {
        if (!std::holds_alternative<bool>(data_->value_)) {
            throw JsonException("GetBoolean() called on non-boolean type");
        }
        return std::get<bool>(data_->value_);
    } catch (const std::bad_variant_access&) {
        throw JsonException("Internal error: invalid type access in GetBoolean()");
    }
}

Json::Impl::Number Json::Impl::GetNumber() const {
    try {
        if (!std::holds_alternative<Number>(data_->value_)) {
            throw JsonException("GetNumber() called on non-number type");
        }
        return std::get<Number>(data_->value_);
    } catch (const std::bad_variant_access&) {
        throw JsonException("Internal error: invalid type access in GetNumber()");
    }
}

const std::string& Json::Impl::GetString() const {
    try {
        if (!std::holds_alternative<std::string>(data_->value_)) {
            throw JsonException("GetString() called on non-string type");
        }
        return std::get<std::string>(data_->value_);
    } catch (const std::bad_variant_access&) {
        throw JsonException("Internal error: invalid type access in GetString()");
    }
}

const Json::Impl::Array& Json::Impl::GetArray() const {
    try {
        if (!std::holds_alternative<Array>(data_->value_)) {
            throw JsonException("GetArray() called on non-array type");
        }
        return std::get<Array>(data_->value_);
    } catch (const std::bad_variant_access&) {
        throw JsonException("Internal error: invalid type access in GetArray()");
    }
}

const Json::Impl::Object& Json::Impl::GetObject() const {
    try {
        if (!std::holds_alternative<Object>(data_->value_)) {
            throw JsonException("GetObject() called on non-object type");
        }
        return std::get<Object>(data_->value_);
    } catch (const std::bad_variant_access&) {
        throw JsonException("Internal error: invalid type access in GetObject()");
    }
}

Json::Impl::Array& Json::Impl::GetArray() {
    EnsureUnique();
    try {
        if (!std::holds_alternative<Array>(data_->value_)) {
            throw JsonException("GetArray() called on non-array type");
        }
        return std::get<Array>(data_->value_);
    } catch (const std::bad_variant_access&) {
        throw JsonException("Internal error: invalid type access in GetArray()");
    }
}

Json::Impl::Object& Json::Impl::GetObject() {
    EnsureUnique();
    try {
        if (!std::holds_alternative<Object>(data_->value_)) {
            throw JsonException("GetObject() called on non-object type");
        }
        return std::get<Object>(data_->value_);
    } catch (const std::bad_variant_access&) {
        throw JsonException("Internal error: invalid type access in GetObject()");
    }
}

void Json::Impl::SetNull() noexcept {
    EnsureUnique();
    data_->value_ = nullptr;
}

void Json::Impl::SetBoolean(bool value) noexcept {
    EnsureUnique();
    data_->value_ = value;
}

void Json::Impl::SetNumber(Number value) noexcept {
    EnsureUnique();
    data_->value_ = value;
}

void Json::Impl::SetString(std::string value) {
    EnsureUnique();
    data_->value_ = std::move(value);
}

void Json::Impl::SetArray() {
    EnsureUnique();
    Array arr;
    arr.reserve(16);  // OPTIMIZATION: Pre-allocate reasonable capacity
    data_->value_ = std::move(arr);
}

void Json::Impl::SetObject() {
    EnsureUnique();
    Object obj;  // SmartObject automatically starts with SmallObject (vector) optimized for ≤4 keys
    data_->value_ = std::move(obj);
}

Json& Json::Impl::At(size_t index) {
    auto& arr = GetArray();
    if (index >= arr.size()) {
        throw JsonException("Array index out of bounds");
    }
    return arr[index];
}

const Json& Json::Impl::At(size_t index) const {
    const auto& arr = GetArray();
    if (index >= arr.size()) {
        throw JsonException("Array index out of bounds");
    }
    return arr[index];
}

void Json::Impl::PushBack(Json value) {
    auto& arr = GetArray();
    
    // OPTIMIZATION: Smart growth prediction to reduce reallocations
    if (arr.size() == arr.capacity()) {
        // When capacity is reached, reserve 2x current size or minimum 32
        size_t new_capacity = std::max(arr.capacity() * 2, size_t(32));
        // Cap maximum auto-reservation to prevent excessive memory usage
        if (new_capacity <= 8192) {
            arr.reserve(new_capacity);
        }
    }
    
    arr.push_back(std::move(value));
}

void Json::Impl::PopBack() {
    auto& arr = GetArray();
    if (arr.empty()) {
        throw JsonException("Cannot pop from empty array");
    }
    arr.pop_back();
}

void Json::Impl::ReserveArray(size_t capacity) {
    GetArray().reserve(capacity);
}

size_t Json::Impl::Size() const noexcept {
    try {
        if (GetType() == Type::Array) {
            return GetArray().size();
        }
        if (GetType() == Type::Object) {
            return GetObject().size();
        }
        return 0;
    } catch (const JsonException&) {
        return 0; // Safe default if variant access fails
    }
}

Json& Json::Impl::operator[](std::string_view key) {
    auto& obj = GetObject();
    
    // OPTIMIZATION: Use string interning for frequently used keys
    std::string key_str;
    if (key.size() <= 32) {  // Only intern reasonably sized keys
        key_str = std::string(InternString(std::string(key)));
    } else {
        key_str = std::string(key);
    }
    
    return obj[key_str];  // SmartObject handles the access pattern optimization
}

const Json& Json::Impl::At(std::string_view key) const {
    const auto& obj = GetObject();
    return obj.at(std::string(key));  // SmartObject handles the lookup
}

bool Json::Impl::Contains(std::string_view key) const noexcept {
    try {
        if (GetType() != Type::Object) return false;
        const auto& obj = GetObject();
        return obj.contains(std::string(key));  // SmartObject handles the lookup
    } catch (const JsonException&) {
        return false; // Safe default if variant access fails
    }
}

void Json::Impl::Remove(std::string_view key) {
    auto& obj = GetObject();
    obj.erase(std::string(key));  // SmartObject handles the removal
}

void Json::Impl::ReserveObject(size_t capacity) {
    GetObject().smart_reserve(capacity);  // Use smart reservation strategy
}

std::vector<std::string> Json::Impl::Keys() const {
    std::vector<std::string> keys;
    const auto& obj = GetObject();
    keys.reserve(obj.size());
    
    // Handle both SmallObject and LargeObject cases
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        keys.push_back(it->first);
    }
    return keys;
}

std::string Json::Impl::ToString(bool pretty) const {
    std::ostringstream ss;
    
    class Printer {
    public:
        explicit Printer(std::ostringstream& ss, bool pretty) 
            : ss_(ss), pretty_(pretty), indent_(0) {}
        
        using Value = std::variant<std::nullptr_t, bool, Number, std::string, Array, Object>;
        
        void Print(const Value& value) {
            std::visit([this](const auto& v) { PrintValue(v); }, value);
        }
        
        void PrintWithCircularCheck(const Impl* impl) {
            // Check for circular reference
            if (visiting_.find(impl) != visiting_.end()) {
                throw JsonException("Circular reference detected during serialization");
            }
            visiting_.insert(impl);
            
            try {
                Print(impl->data_->value_);
            } catch (...) {
                visiting_.erase(impl);
                throw;
            }
            
            visiting_.erase(impl);
        }
        
    private:
        void PrintIndent() {
            if (pretty_) {
                ss_ << std::string(indent_ * 2, ' ');
            }
        }
        
        void PrintNewline() {
            if (pretty_) {
                ss_ << '\n';
            }
        }
        
        void PrintValue(std::nullptr_t) {
            ss_ << "null";
        }
        
        void PrintValue(bool value) {
            ss_ << (value ? "true" : "false");
        }
        
        void PrintValue(Number value) {
            ss_ << std::setprecision(17) << value;
        }
        
        void PrintValue(const std::string& value) {
            ss_ << '"';
            for (char c : value) {
                switch (c) {
                    case '"': ss_ << "\\\""; break;
                    case '\\': ss_ << "\\\\"; break;
                    case '\b': ss_ << "\\b"; break;
                    case '\f': ss_ << "\\f"; break;
                    case '\n': ss_ << "\\n"; break;
                    case '\r': ss_ << "\\r"; break;
                    case '\t': ss_ << "\\t"; break;
                    default:
                        if (static_cast<unsigned char>(c) < 0x20) {
                            ss_ << "\\u" << std::hex << std::setw(4) 
                                << std::setfill('0') << static_cast<int>(c);
                        } else {
                            ss_ << c;
                        }
                }
            }
            ss_ << '"';
        }
        
        void PrintValue(const Array& arr) {
            ss_ << '[';
            if (!arr.empty()) {
                PrintNewline();
                ++indent_;
                for (size_t i = 0; i < arr.size(); ++i) {
                    PrintIndent();
                    PrintWithCircularCheck(arr[i].impl_.get());
                    if (i < arr.size() - 1) {
                        ss_ << ',';
                    }
                    PrintNewline();
                }
                --indent_;
                PrintIndent();
            }
            ss_ << ']';
        }
        
        void PrintValue(const Object& obj) {
            ss_ << '{';
            if (!obj.empty()) {
                PrintNewline();
                ++indent_;
                size_t i = 0;
                for (const auto& [key, value] : obj) {
                    PrintIndent();
                    PrintValue(key);
                    ss_ << (pretty_ ? ": " : ":");
                    PrintWithCircularCheck(value.impl_.get());
                    if (i < obj.size() - 1) {
                        ss_ << ',';
                    }
                    PrintNewline();
                    ++i;
                }
                --indent_;
                PrintIndent();
            }
            ss_ << '}';
        }
        
        std::ostringstream& ss_;
        bool pretty_;
        size_t indent_;
        std::unordered_set<const Impl*> visiting_;
    };
    
    Printer printer(ss, pretty);
    printer.PrintWithCircularCheck(this);
    return ss.str();
}
