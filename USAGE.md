# JSON Library Usage Guide

This guide shows how to build and use the JSON library using the provided Makefile.

## Building the Library

### Basic Build Commands

```bash
# Build release libraries (default)
make
# or
make release

# Build debug libraries
make debug

# Build both debug and release
make both
```

### What Gets Built

The Makefile creates both static and shared libraries:

**Release build:**
- `build/release/libjson.a` - Static library
- `build/release/libjson.so.1.0.0` - Shared library (versioned)
- `build/release/libjson.so.1` - Shared library (major version symlink)
- `build/release/libjson.so` - Shared library (development symlink)

**Debug build:**
- `build/debug/libjson.a` - Static library (with debug symbols)
- `build/debug/libjson.so.1.0.0` - Shared library (with debug symbols)

## Using the Library

### Static Linking

```bash
# Build with static library
g++ -std=c++20 -I/path/to/headers my_program.cpp /path/to/libjson.a -o my_program

# Using make example
make example-static
./example_static
```

### Dynamic Linking

```bash
# Build with shared library
g++ -std=c++20 -I/path/to/headers my_program.cpp -L/path/to/lib -ljson -o my_program

# Run with library path
LD_LIBRARY_PATH=/path/to/lib ./my_program

# Using make example
make example-shared
LD_LIBRARY_PATH=build/release ./example_shared
```

## Installation

### System Installation

```bash
# Install to /usr/local (requires sudo)
sudo make install

# After installation, you can use the library system-wide:
g++ -std=c++20 my_program.cpp -ljson -o my_program
```

### Custom Installation Location

```bash
# Install to custom location
make install INSTALL_PREFIX=/opt/json-library
```

### Uninstall

```bash
sudo make uninstall
```

## Testing

```bash
# Run all tests
make test

# Run specific tests
make test-comprehensive
make test-iter
make test-remove
make test-serial
make test-debug
```

## Packaging

```bash
# Create distribution package
make dist
# Creates json-library-1.0.0.tar.gz
```

## Maintenance

```bash
# Clean build artifacts
make clean

# Clean everything including distribution files
make distclean

# Show library information
make info

# Show help
make help
```

## Integration with Your Project

### CMake Integration

After installation, you can use the library in your CMake project:

```cmake
find_library(JSON_LIBRARY json)
find_path(JSON_INCLUDE_DIR Json.h)

target_include_directories(your_target PRIVATE ${JSON_INCLUDE_DIR})
target_link_libraries(your_target ${JSON_LIBRARY})
```

### pkg-config Support

You can create a `.pc` file for pkg-config integration:

```bash
# Create json.pc file in /usr/local/lib/pkgconfig/
cat > json.pc << EOF
prefix=/usr/local
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: json
Description: Modern C++20 JSON Library
Version: 1.0.0
Libs: -L\${libdir} -ljson
Cflags: -I\${includedir} -std=c++20
EOF
```

Then use it in your build:

```bash
g++ `pkg-config --cflags --libs json` my_program.cpp -o my_program
```

## Example Usage in Code

```cpp
#include <Json.h>
#include <iostream>

int main() {
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
    
    return 0;
}
```

## Library Features

- ✅ Complete JSON types support (null, bool, number, string, array, object)
- ✅ Modern C++20 with concepts and ranges
- ✅ STL-style iterators
- ✅ Structured bindings support
- ✅ Template-based type-safe access
- ✅ Exception-based error handling
- ✅ Memory-safe RAII design
- ✅ Both static and shared library builds
- ✅ Comprehensive test suite 