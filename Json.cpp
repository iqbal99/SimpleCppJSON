#include "Json.h"
#include "JsonImpl.h"
#include <sstream>
#include <charconv>
#include <cctype>
#include <map>

// Constructors
Json::Json() noexcept : impl_(Impl::AcquireImpl()) {}
Json::Json(std::nullptr_t) noexcept : impl_(Impl::AcquireImpl()) {}
Json::Json(bool value) noexcept : impl_(Impl::AcquireImpl()) { impl_->SetBoolean(value); }
Json::Json(int value) noexcept : impl_(Impl::AcquireImpl()) { impl_->SetNumber(static_cast<double>(value)); }
Json::Json(int64_t value) noexcept : impl_(Impl::AcquireImpl()) { impl_->SetNumber(static_cast<double>(value)); }
Json::Json(double value) noexcept : impl_(Impl::AcquireImpl()) { impl_->SetNumber(value); }
Json::Json(const char* value) : impl_(Impl::AcquireImpl()) { impl_->SetString(std::string(value)); }
Json::Json(std::string_view value) : impl_(Impl::AcquireImpl()) { impl_->SetString(std::string(value)); }
Json::Json(std::string value) : impl_(Impl::AcquireImpl()) { impl_->SetString(std::move(value)); }

// Copy operations
Json::Json(const Json& other) : impl_(Impl::AcquireImpl()) { *impl_ = *other.impl_; }

Json& Json::operator=(const Json& other) {
    if (this != &other) {
        *impl_ = *other.impl_;
    }
    return *this;
}

// Move operations
Json::Json(Json&& other) noexcept = default;
Json& Json::operator=(Json&& other) noexcept = default;

// Destructor with memory pool integration
Json::~Json() {
    if (impl_) {
        // Create a temporary unique_ptr to move the impl into the pool
        auto temp_impl = std::move(impl_);
        Impl::ReleaseImpl(std::move(temp_impl));
    }
}

// Factory methods
Json Json::Array() {
    Json json;
    json.impl_ = Impl::AcquireImpl();
    json.impl_->SetArray();
    return json;
}

Json Json::Object() {
    Json json;
    json.impl_ = Impl::AcquireImpl();
    json.impl_->SetObject();
    return json;
}

// JSON Parser implementation
class JsonParser {
private:
    std::string_view input_;
    size_t pos_;
    size_t line_;
    size_t column_;

public:
    explicit JsonParser(std::string_view input) 
        : input_(input), pos_(0), line_(1), column_(1) {}

    Json Parse() {
        SkipWhitespace();
        if (pos_ >= input_.size()) {
            throw JsonParseError("Unexpected end of input", line_, column_);
        }
        
        Json result = ParseValue();
        SkipWhitespace();
        
        if (pos_ < input_.size()) {
            throw JsonParseError("Extra content after JSON", line_, column_);
        }
        
        return result;
    }

private:
    char Current() const {
        return pos_ < input_.size() ? input_[pos_] : '\0';
    }

    char Advance() {
        if (pos_ < input_.size()) {
            char c = input_[pos_++];
            if (c == '\n') {
                line_++;
                column_ = 1;
            } else {
                column_++;
            }
            return c;
        }
        return '\0';
    }

    void SkipWhitespace() {
        while (pos_ < input_.size() && std::isspace(input_[pos_])) {
            Advance();
        }
    }

    Json ParseValue() {
        SkipWhitespace();
        
        if (pos_ >= input_.size()) {
            throw JsonParseError("Unexpected end of input", line_, column_);
        }

        char c = Current();
        switch (c) {
            case 'n': return ParseNull();
            case 't': case 'f': return ParseBoolean();
            case '"': return ParseString();
            case '[': return ParseArray();
            case '{': return ParseObject();
            case '-': case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                return ParseNumber();
            default:
                throw JsonParseError("Unexpected character: " + std::string(1, c), line_, column_);
        }
    }

    Json ParseNull() {
        if (input_.substr(pos_, 4) == "null") {
            pos_ += 4;
            column_ += 4;
            return Json(nullptr);
        }
        throw JsonParseError("Invalid null literal", line_, column_);
    }

    Json ParseBoolean() {
        if (input_.substr(pos_, 4) == "true") {
            pos_ += 4;
            column_ += 4;
            return Json(true);
        }
        if (input_.substr(pos_, 5) == "false") {
            pos_ += 5;
            column_ += 5;
            return Json(false);
        }
        throw JsonParseError("Invalid boolean literal", line_, column_);
    }

    Json ParseString() {
        if (Current() != '"') {
            throw JsonParseError("Expected '\"'", line_, column_);
        }
        
        Advance(); // Skip opening quote
        std::string result;
        
        while (pos_ < input_.size() && Current() != '"') {
            char c = Current();
            if (c == '\\') {
                Advance();
                if (pos_ >= input_.size()) {
                    throw JsonParseError("Unterminated string escape", line_, column_);
                }
                
                char escaped = Current();
                switch (escaped) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    case 'u': {
                        // Unicode escape sequence
                        if (pos_ + 4 >= input_.size()) {
                            throw JsonParseError("Incomplete unicode escape", line_, column_);
                        }
                        std::string hexStr = std::string(input_.substr(pos_ + 1, 4));
                        
                        // Validate that all 4 characters are hexadecimal
                        for (char c : hexStr) {
                            if (!std::isxdigit(c)) {
                                throw JsonParseError("Invalid unicode escape", line_, column_);
                            }
                        }
                        
                        try {
                            unsigned int codepoint = std::stoul(hexStr, nullptr, 16);
                            // For simplicity, only handle ASCII range
                            if (codepoint <= 0x7F) {
                                result += static_cast<char>(codepoint);
                            } else {
                                result += '?'; // Replace with placeholder
                            }
                        } catch (const std::exception&) {
                            throw JsonParseError("Invalid unicode escape", line_, column_);
                        }
                        pos_ += 4;
                        column_ += 4;
                        break;
                    }
                    default:
                        throw JsonParseError("Invalid escape sequence", line_, column_);
                }
                Advance();
            } else if (c < 0x20) {
                throw JsonParseError("Invalid character in string", line_, column_);
            } else {
                result += c;
                Advance();
            }
        }
        
        if (Current() != '"') {
            throw JsonParseError("Unterminated string", line_, column_);
        }
        
        Advance(); // Skip closing quote
        return Json(std::move(result));
    }

    Json ParseNumber() {
        size_t start = pos_;
        
        if (Current() == '-') {
            Advance();
        }
        
        if (!std::isdigit(Current())) {
            throw JsonParseError("Invalid number", line_, column_);
        }
        
        if (Current() == '0') {
            Advance();
        } else {
            while (std::isdigit(Current())) {
                Advance();
            }
        }
        
        if (Current() == '.') {
            Advance();
            if (!std::isdigit(Current())) {
                throw JsonParseError("Invalid number", line_, column_);
            }
            while (std::isdigit(Current())) {
                Advance();
            }
        }
        
        if (Current() == 'e' || Current() == 'E') {
            Advance();
            if (Current() == '+' || Current() == '-') {
                Advance();
            }
            if (!std::isdigit(Current())) {
                throw JsonParseError("Invalid number", line_, column_);
            }
            while (std::isdigit(Current())) {
                Advance();
            }
        }
        
        std::string numberStr(input_.substr(start, pos_ - start));
        double value = std::stod(numberStr);
        return Json(value);
    }

    Json ParseArray() {
        if (Current() != '[') {
            throw JsonParseError("Expected '['", line_, column_);
        }
        
        Advance();
        Json array = Json::Array();
        SkipWhitespace();
        
        if (Current() == ']') {
            Advance();
            return array;
        }
        
        while (true) {
            array.PushBack(ParseValue());
            SkipWhitespace();
            
            if (Current() == ']') {
                Advance();
                break;
            } else if (Current() == ',') {
                Advance();
                SkipWhitespace();
            } else {
                throw JsonParseError("Expected ',' or ']'", line_, column_);
            }
        }
        
        return array;
    }

    Json ParseObject() {
        if (Current() != '{') {
            throw JsonParseError("Expected '{'", line_, column_);
        }
        
        Advance();
        Json object = Json::Object();
        SkipWhitespace();
        
        if (Current() == '}') {
            Advance();
            return object;
        }
        
        while (true) {
            SkipWhitespace();
            if (Current() != '"') {
                throw JsonParseError("Expected string key", line_, column_);
            }
            
            Json key = ParseString();
            std::string keyStr = key.Get<std::string>();
            
            SkipWhitespace();
            if (Current() != ':') {
                throw JsonParseError("Expected ':'", line_, column_);
            }
            Advance();
            
            object[keyStr] = ParseValue();
            SkipWhitespace();
            
            if (Current() == '}') {
                Advance();
                break;
            } else if (Current() == ',') {
                Advance();
            } else {
                throw JsonParseError("Expected ',' or '}'", line_, column_);
            }
        }
        
        return object;
    }
};

Json Json::Parse(std::string_view json_string) {
    JsonParser parser(json_string);
    return parser.Parse();
}

// Type checking
bool Json::IsNull() const noexcept { 
    if (!impl_) return false; // Safe default for moved-from objects
    return impl_->GetType() == Type::Null; 
}
bool Json::IsBoolean() const noexcept { 
    if (!impl_) return false; // Safe default for moved-from objects
    return impl_->GetType() == Type::Boolean; 
}
bool Json::IsNumber() const noexcept { 
    if (!impl_) return false; // Safe default for moved-from objects
    return impl_->GetType() == Type::Number; 
}
bool Json::IsString() const noexcept { 
    if (!impl_) return false; // Safe default for moved-from objects
    return impl_->GetType() == Type::String; 
}
bool Json::IsArray() const noexcept { 
    if (!impl_) return false; // Safe default for moved-from objects
    return impl_->GetType() == Type::Array; 
}
bool Json::IsObject() const noexcept { 
    if (!impl_) return false; // Safe default for moved-from objects
    return impl_->GetType() == Type::Object; 
}
Json::Type Json::GetType() const noexcept { 
    if (!impl_) return Type::Null; // Safe default for moved-from objects
    return impl_->GetType(); 
}

// Array operations
Json& Json::operator[](size_t index) {
    ensure_valid();
    return impl_->At(index);
}

const Json& Json::operator[](size_t index) const {
    ensure_valid();
    return impl_->At(index);
}

void Json::PushBack(Json value) {
    ensure_valid();
    impl_->PushBack(std::move(value));
}

void Json::PopBack() {
    ensure_valid();
    impl_->PopBack();
}

void Json::Reserve(size_t capacity) {
    if (IsArray()) {
        impl_->ReserveArray(capacity);
    } else if (IsObject()) {
        impl_->ReserveObject(capacity);
    }
}

size_t Json::Size() const {
    ensure_valid();
    if (!IsArray() && !IsObject()) {
        throw JsonException("Size() can only be called on arrays or objects");
    }
    return impl_->Size();
}

// Object operations
Json& Json::operator[](std::string_view key) {
    ensure_valid();
    return (*impl_)[key];
}

const Json& Json::operator[](std::string_view key) const {
    ensure_valid();
    return impl_->At(key);
}

bool Json::Contains(std::string_view key) const {
    ensure_valid();
    if (!IsObject()) {
        throw JsonException("Contains() can only be called on objects");
    }
    return impl_->Contains(key);
}

void Json::Remove(std::string_view key) {
    ensure_valid();
    impl_->Remove(key);
}

std::vector<std::string> Json::Keys() const {
    ensure_valid();
    return impl_->Keys();
}

// Serialization
std::string Json::ToString(bool pretty) const {
    ensure_valid();
    return impl_->ToString(pretty);
}

// Exception implementations
// Validity check implementation
void Json::ensure_valid() const {
    if (!impl_) {
        throw JsonException("Operation on moved-from or invalid Json object");
    }
}

JsonException::JsonException(const std::string& message) : std::runtime_error(message) {}
JsonException::JsonException(const char* message) : std::runtime_error(message) {}

JsonTypeError::JsonTypeError(Json::Type expected, Json::Type actual) 
    : JsonException("Type mismatch: expected " + std::to_string(static_cast<int>(expected)) + 
                   ", got " + std::to_string(static_cast<int>(actual))) {}

JsonParseError::JsonParseError(const std::string& message, size_t line, size_t column)
    : JsonException(message + " at line " + std::to_string(line) + ", column " + std::to_string(column))
    , line_(line)
    , column_(column) {}

size_t JsonParseError::Line() const noexcept { return line_; }
size_t JsonParseError::Column() const noexcept { return column_; }

// Array Iterator implementations
Json::Iterator::Iterator() noexcept : impl_(nullptr), index_(0) {}

Json::Iterator::Iterator(Impl* impl, size_t index) : impl_(impl), index_(index) {}

Json::Iterator& Json::Iterator::operator++() {
    ++index_;
    return *this;
}

Json::Iterator Json::Iterator::operator++(int) {
    Iterator tmp(*this);
    ++*this;
    return tmp;
}

Json::Iterator& Json::Iterator::operator--() {
    --index_;
    return *this;
}

Json::Iterator Json::Iterator::operator--(int) {
    Iterator tmp(*this);
    --*this;
    return tmp;
}

Json::Iterator::reference Json::Iterator::operator*() const {
    return impl_->At(index_);
}

Json::Iterator::pointer Json::Iterator::operator->() const {
    return &impl_->At(index_);
}

bool Json::Iterator::operator==(const Iterator& other) const noexcept {
    return impl_ == other.impl_ && index_ == other.index_;
}

bool Json::Iterator::operator!=(const Iterator& other) const noexcept {
    return !(*this == other);
}

// Const Array Iterator implementations
Json::ConstIterator::ConstIterator() noexcept : impl_(nullptr), index_(0) {}

Json::ConstIterator::ConstIterator(const Iterator& it) noexcept 
    : impl_(it.impl_), index_(it.index_) {}

Json::ConstIterator::ConstIterator(const Impl* impl, size_t index) 
    : impl_(impl), index_(index) {}

Json::ConstIterator& Json::ConstIterator::operator++() {
    ++index_;
    return *this;
}

Json::ConstIterator Json::ConstIterator::operator++(int) {
    ConstIterator tmp(*this);
    ++*this;
    return tmp;
}

Json::ConstIterator& Json::ConstIterator::operator--() {
    --index_;
    return *this;
}

Json::ConstIterator Json::ConstIterator::operator--(int) {
    ConstIterator tmp(*this);
    --*this;
    return tmp;
}

Json::ConstIterator::reference Json::ConstIterator::operator*() const {
    return impl_->At(index_);
}

Json::ConstIterator::pointer Json::ConstIterator::operator->() const {
    return &impl_->At(index_);
}

bool Json::ConstIterator::operator==(const ConstIterator& other) const noexcept {
    return impl_ == other.impl_ && index_ == other.index_;
}

bool Json::ConstIterator::operator!=(const ConstIterator& other) const noexcept {
    return !(*this == other);
}

// Object Iterator Implementation classes
class Json::ObjectIterator::ObjectIterImpl {
public:
    Json::Impl::Object::iterator it_;
    Json::Impl::Object::iterator end_it_;  // Store the end iterator for comparison
    Json::KeyValue current_;
    bool valid_;
    Json dummy_json_;  // Dummy Json object for invalid iterators

    ObjectIterImpl() : current_("", dummy_json_), valid_(false) {}
    
    ObjectIterImpl(Json::Impl::Object::iterator it, Json::Impl::Object::iterator end_it) 
        : it_(it), end_it_(end_it), current_("", dummy_json_), valid_(it != end_it) {
        Update();
    }
    
    void Update() {
        if (valid_ && it_ != end_it_) {
            current_.key = it_->first;
            current_.value_ptr = &it_->second;
        } else {
            valid_ = false;
        }
    }
};

class Json::ConstObjectIterator::ConstObjectIterImpl {
public:
    Json::Impl::Object::const_iterator it_;
    Json::Impl::Object::const_iterator end_it_;  // Store the end iterator for comparison
    Json::ConstKeyValue current_;
    bool valid_;
    Json dummy_json_;  // Dummy Json object for invalid iterators

    ConstObjectIterImpl() : current_("", dummy_json_), valid_(false) {}
    
    ConstObjectIterImpl(Json::Impl::Object::const_iterator it, Json::Impl::Object::const_iterator end_it) 
        : it_(it), end_it_(end_it), current_("", dummy_json_), valid_(it != end_it) {
        Update();
    }
    
    void Update() {
        if (valid_ && it_ != end_it_) {
            current_.key = it_->first;
            current_.value_ptr = &it_->second;
        } else {
            valid_ = false;
        }
    }
};

// Object Iterator implementations
Json::ObjectIterator::ObjectIterator() noexcept 
    : impl_(std::make_unique<ObjectIterImpl>()) {}

Json::ObjectIterator::ObjectIterator(const ObjectIterator& other) 
    : impl_(std::make_unique<ObjectIterImpl>(*other.impl_)) {}

Json::ObjectIterator::ObjectIterator(ObjectIterator&& other) noexcept 
    : impl_(std::move(other.impl_)) {}

Json::ObjectIterator& Json::ObjectIterator::operator=(const ObjectIterator& other) {
    if (this != &other) {
        impl_ = std::make_unique<ObjectIterImpl>(*other.impl_);
    }
    return *this;
}

Json::ObjectIterator& Json::ObjectIterator::operator=(ObjectIterator&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

Json::ObjectIterator::~ObjectIterator() = default;

Json::ObjectIterator::ObjectIterator(Impl* json_impl) 
    : impl_(std::make_unique<ObjectIterImpl>(json_impl->GetObject().begin(), json_impl->GetObject().end())) {}

Json::ObjectIterator::ObjectIterator(Impl* json_impl, bool end) {
    if (end) {
        impl_ = std::make_unique<ObjectIterImpl>();
        impl_->it_ = json_impl->GetObject().end();
        impl_->end_it_ = json_impl->GetObject().end();
        impl_->valid_ = false;
    } else {
        impl_ = std::make_unique<ObjectIterImpl>(json_impl->GetObject().begin(), json_impl->GetObject().end());
    }
}

Json::ObjectIterator& Json::ObjectIterator::operator++() {
    if (impl_->valid_) {
        ++impl_->it_;
        impl_->Update();
    }
    return *this;
}

Json::ObjectIterator Json::ObjectIterator::operator++(int) {
    ObjectIterator tmp(*this);
    ++*this;
    return tmp;
}

Json::ObjectIterator::reference Json::ObjectIterator::operator*() {
    return impl_->current_;
}

Json::KeyValue* Json::ObjectIterator::operator->() {
    return &impl_->current_;
}

bool Json::ObjectIterator::operator==(const ObjectIterator& other) const noexcept {
    return impl_->it_ == other.impl_->it_;
}

bool Json::ObjectIterator::operator!=(const ObjectIterator& other) const noexcept {
    return !(*this == other);
}

// Const Object Iterator implementations
Json::ConstObjectIterator::ConstObjectIterator() noexcept 
    : impl_(std::make_unique<ConstObjectIterImpl>()) {}

Json::ConstObjectIterator::ConstObjectIterator(const ConstObjectIterator& other) 
    : impl_(std::make_unique<ConstObjectIterImpl>(*other.impl_)) {}

Json::ConstObjectIterator::ConstObjectIterator(ConstObjectIterator&& other) noexcept 
    : impl_(std::move(other.impl_)) {}

Json::ConstObjectIterator& Json::ConstObjectIterator::operator=(const ConstObjectIterator& other) {
    if (this != &other) {
        impl_ = std::make_unique<ConstObjectIterImpl>(*other.impl_);
    }
    return *this;
}

Json::ConstObjectIterator& Json::ConstObjectIterator::operator=(ConstObjectIterator&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

Json::ConstObjectIterator::~ConstObjectIterator() = default;

Json::ConstObjectIterator::ConstObjectIterator(const ObjectIterator& it) noexcept
    : impl_(std::make_unique<ConstObjectIterImpl>()) {
    impl_->it_ = it.impl_->it_;
    impl_->end_it_ = it.impl_->end_it_;
    impl_->valid_ = it.impl_->valid_;
    if (impl_->valid_) {
        impl_->Update();
    }
}

Json::ConstObjectIterator::ConstObjectIterator(const Impl* json_impl) 
    : impl_(std::make_unique<ConstObjectIterImpl>(json_impl->GetObject().begin(), json_impl->GetObject().end())) {}

Json::ConstObjectIterator::ConstObjectIterator(const Impl* json_impl, bool end) {
    if (end) {
        impl_ = std::make_unique<ConstObjectIterImpl>();
        impl_->it_ = json_impl->GetObject().end();
        impl_->end_it_ = json_impl->GetObject().end();
        impl_->valid_ = false;
    } else {
        impl_ = std::make_unique<ConstObjectIterImpl>(json_impl->GetObject().begin(), json_impl->GetObject().end());
    }
}

Json::ConstObjectIterator& Json::ConstObjectIterator::operator++() {
    if (impl_->valid_) {
        ++impl_->it_;
        impl_->Update();
    }
    return *this;
}

Json::ConstObjectIterator Json::ConstObjectIterator::operator++(int) {
    ConstObjectIterator tmp(*this);
    ++*this;
    return tmp;
}

Json::ConstObjectIterator::reference Json::ConstObjectIterator::operator*() {
    return impl_->current_;
}

Json::ConstKeyValue* Json::ConstObjectIterator::operator->() {
    return &impl_->current_;
}

bool Json::ConstObjectIterator::operator==(const ConstObjectIterator& other) const noexcept {
    return impl_->it_ == other.impl_->it_;
}

bool Json::ConstObjectIterator::operator!=(const ConstObjectIterator& other) const noexcept {
    return !(*this == other);
}

// Array iteration methods
Json::Iterator Json::begin() noexcept {
    if (!impl_) return Iterator(nullptr, 0); // Safe default for moved-from objects
    if (!IsArray() && !IsObject()) return Iterator(impl_.get(), 0); // Empty range for non-arrays/objects
    return Iterator(impl_.get(), 0);
}

Json::Iterator Json::end() noexcept {
    if (!impl_) return Iterator(nullptr, 0); // Safe default for moved-from objects
    if (!IsArray() && !IsObject()) return Iterator(impl_.get(), 0); // Empty range for non-arrays/objects
    return Iterator(impl_.get(), Size());
}

Json::ConstIterator Json::begin() const noexcept {
    if (!impl_) return ConstIterator(nullptr, 0); // Safe default for moved-from objects
    if (!IsArray() && !IsObject()) return ConstIterator(impl_.get(), 0); // Empty range for non-arrays/objects
    return ConstIterator(impl_.get(), 0);
}

Json::ConstIterator Json::end() const noexcept {
    if (!impl_) return ConstIterator(nullptr, 0); // Safe default for moved-from objects
    if (!IsArray() && !IsObject()) return ConstIterator(impl_.get(), 0); // Empty range for non-arrays/objects
    return ConstIterator(impl_.get(), Size());
}

Json::ConstIterator Json::cbegin() const noexcept {
    return begin();
}

Json::ConstIterator Json::cend() const noexcept {
    return end();
}

// Object iteration methods
Json::ObjectIterator Json::object_begin() noexcept {
    if (!impl_) return ObjectIterator(); // Safe default for moved-from objects
    return ObjectIterator(impl_.get());
}

Json::ObjectIterator Json::object_end() noexcept {
    if (!impl_) return ObjectIterator(); // Safe default for moved-from objects
    return ObjectIterator(impl_.get(), true);
}

Json::ConstObjectIterator Json::object_begin() const noexcept {
    if (!impl_) return ConstObjectIterator(); // Safe default for moved-from objects
    return ConstObjectIterator(impl_.get());
}

Json::ConstObjectIterator Json::object_end() const noexcept {
    if (!impl_) return ConstObjectIterator(); // Safe default for moved-from objects
    return ConstObjectIterator(impl_.get(), true);
}

Json::ConstObjectIterator Json::object_cbegin() const noexcept {
    return object_begin();
}

Json::ConstObjectIterator Json::object_cend() const noexcept {
    return object_end();
}

// Range helper implementations
Json::ObjectIterator Json::ObjectRange::begin() noexcept {
    if (!json_.IsObject()) {
        // Return empty iterator instead of crashing
        return ObjectIterator();
    }
    return json_.object_begin();
}

Json::ObjectIterator Json::ObjectRange::end() noexcept {
    if (!json_.IsObject()) {
        return ObjectIterator(); // Empty range
    }
    return json_.object_end();
}

Json::ConstObjectIterator Json::ConstObjectRange::begin() const noexcept {
    if (!json_.IsObject()) {
        // Return empty iterator instead of crashing
        return ConstObjectIterator();
    }
    return json_.object_begin();
}

Json::ConstObjectIterator Json::ConstObjectRange::end() const noexcept {
    if (!json_.IsObject()) {
        return ConstObjectIterator(); // Empty range
    }
    return json_.object_end();
}

// Template method definitions (moved from JsonTemplates.h)
template<typename T>
requires std::same_as<T, bool> ||
         std::integral<T> ||
         std::floating_point<T> ||
         std::convertible_to<T, std::string_view>
T Json::Get() const {
    ensure_valid(); // Add validity check
    if constexpr (std::same_as<T, bool>) {
        if (!IsBoolean()) throw JsonTypeError(Type::Boolean, GetType());
        return impl_->GetBoolean();
    }
    else if constexpr (std::integral<T>) {
        if (!IsNumber()) throw JsonTypeError(Type::Number, GetType());
        return static_cast<T>(impl_->GetNumber());
    }
    else if constexpr (std::floating_point<T>) {
        if (!IsNumber()) throw JsonTypeError(Type::Number, GetType());
        return static_cast<T>(impl_->GetNumber());
    }
    else if constexpr (std::convertible_to<T, std::string_view>) {
        if (!IsString()) throw JsonTypeError(Type::String, GetType());
        return impl_->GetString();
    }
}

template<typename T>
requires std::same_as<T, bool> ||
         std::integral<T> ||
         std::floating_point<T> ||
         std::convertible_to<T, std::string_view>
void Json::Set(T value) {
    ensure_valid(); // Add validity check
    if constexpr (std::same_as<T, bool>) {
        impl_->SetBoolean(value);
    }
    else if constexpr (std::integral<T> || std::floating_point<T>) {
        impl_->SetNumber(static_cast<double>(value));
    }
    else if constexpr (std::convertible_to<T, std::string_view>) {
        impl_->SetString(std::string(value));
    }
}

template<typename T>
std::optional<T> Json::TryGet() const noexcept {
    try {
        if (!impl_) return std::nullopt; // Safe default for moved-from objects
        return Get<T>();
    } catch (const JsonTypeError&) {
        return std::nullopt;
    } catch (const JsonException&) { // Catch validity check exceptions too
        return std::nullopt;
    }
}

// Explicit template instantiations for commonly used types
// This ensures the templates are compiled into the library

// Get<T> instantiations
template bool Json::Get<bool>() const;
template int Json::Get<int>() const;
template long Json::Get<long>() const;
template long long Json::Get<long long>() const;
template unsigned int Json::Get<unsigned int>() const;
template unsigned long Json::Get<unsigned long>() const;
template unsigned long long Json::Get<unsigned long long>() const;
template float Json::Get<float>() const;
template double Json::Get<double>() const;
template std::string Json::Get<std::string>() const;

// Set<T> instantiations
template void Json::Set<bool>(bool);
template void Json::Set<int>(int);
template void Json::Set<long>(long);
template void Json::Set<long long>(long long);
template void Json::Set<unsigned int>(unsigned int);
template void Json::Set<unsigned long>(unsigned long);
template void Json::Set<unsigned long long>(unsigned long long);
template void Json::Set<float>(float);
template void Json::Set<double>(double);
template void Json::Set<std::string>(std::string);
template void Json::Set<const char*>(const char*);
template void Json::Set<std::string_view>(std::string_view);

// TryGet<T> instantiations
template std::optional<bool> Json::TryGet<bool>() const noexcept;
template std::optional<int> Json::TryGet<int>() const noexcept;
template std::optional<long> Json::TryGet<long>() const noexcept;
template std::optional<long long> Json::TryGet<long long>() const noexcept;
template std::optional<unsigned int> Json::TryGet<unsigned int>() const noexcept;
template std::optional<unsigned long> Json::TryGet<unsigned long>() const noexcept;
template std::optional<unsigned long long> Json::TryGet<unsigned long long>() const noexcept;
template std::optional<float> Json::TryGet<float>() const noexcept;
template std::optional<double> Json::TryGet<double>() const noexcept;
template std::optional<std::string> Json::TryGet<std::string>() const noexcept;
