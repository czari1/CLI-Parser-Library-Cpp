# ArgParser

A modern C++20 argument parser library that provides a clean and intuitive API for handling command-line arguments in your applications.

## Features

- **Modern C++20**: Takes advantage of the latest C++ features including concepts, string_view, and optional
- **Type Safety**: Template-based type system with compile-time checks
- **Flexible Arguments**: Support for flags, options, and positional arguments
- **Validation**: Built-in argument validation with custom validator functions
- **Automatic Help**: Generated help messages with proper formatting
- **Exception Safety**: Comprehensive error handling with custom exception types
- **Easy Integration**: Header-only design with simple CMake integration

## Quick Start

### Basic Example

```cpp
#include "ArgParser.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        argparser::ArgParser parser("myapp", "A sample application");
        
        // Add a flag
        parser.addFlag("v", "verbose", "Enable verbose output");
        
        // Add an option with default value
        parser.addOption("o", "output", "Output file", "output.txt");
        
        // Add a required positional argument
        parser.addPositional("input", "Input file", true);
        
        // Parse arguments
        parser.parseOptions(argc, argv);
        
        // Use the arguments
        bool verbose = parser.getBool("verbose");
        std::string output = parser.getString("output");
        std::string input = parser.getString("input");
        
        if (verbose) {
            std::cout << "Processing " << input << " -> " << output << std::endl;
        }
        
    } catch (const argparser::ArgumentError& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### Advanced Example

```cpp
argparser::ArgParser parser;

parser.programName("advanced-app")
      .description("An advanced example application")
      .version("1.0.0");

// Flag with validation
parser.addFlag("d", "debug", "Enable debug mode");

// Option with custom validator
auto& portArg = parser.addOption("p", "port", "Server port", "8080");
portArg.validator([](const std::string& value) {
    try {
        int port = std::stoi(value);
        return port > 0 && port < 65536;
    } catch (...) {
        return false;
    }
});

// Required option
parser.addOption("c", "config", "Configuration file")
      .required(true);

// Optional positional argument
parser.addPositional("files", "Input files", false);

parser.parseOptions(argc, argv);

// Type-safe value retrieval
auto port = parser.get<int>("port");
if (port) {
    std::cout << "Server will run on port: " << *port << std::endl;
}
```

## Building

### Requirements

- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)
- CMake 3.16 or higher

### CMake Integration

```cmake
# Add the library to your project
add_subdirectory(argparser)

# Link against your target
target_link_libraries(your_target argparser::argparser)
```

### Manual Build

```bash
mkdir build
cd build
cmake ..
make
```

## API Reference

### ArgParser Class

#### Adding Arguments

**Flags** (boolean arguments):
```cpp
Argument& addFlag(std::string_view shortName, 
                  std::string_view longName,
                  std::string_view description);
```

**Options** (arguments with values):
```cpp
Argument& addOption(std::string_view shortName, 
                    std::string_view longName,
                    std::string_view description,
                    std::string_view defaultValue = "");
```

**Positional Arguments**:
```cpp
Argument& addPositional(std::string_view name, 
                        std::string_view description,
                        bool required = false);
```

#### Parsing
```cpp
void parseOptions(int argc, char* argv[]);
```

#### Value Retrieval

**Generic template method**:
```cpp
template<typename T>
std::optional<T> get(std::string_view name) const;
```

**Type-specific methods**:
```cpp
std::string getString(std::string_view name) const;
int getInt(std::string_view name) const;
double getDouble(std::string_view name) const;
bool getBool(std::string_view name) const;
```

**Check if argument was provided**:
```cpp
bool isSet(std::string_view name) const;
```

#### Configuration
```cpp
ArgParser& programName(std::string_view name);
ArgParser& description(std::string_view desc);
ArgParser& version(std::string_view version);
```

#### Help System
```cpp
std::string help() const;
void printHelp() const;
```

### Argument Class

#### Chaining Methods
```cpp
Argument& required(bool isRequired = true);
Argument& defaultValue(std::string_view value);
Argument& help(std::string_view description);
Argument& validator(ValidatorFunction func);
```

## Supported Argument Types

| Type | Description | Example |
|------|-------------|---------|
| `std::string` | Text values | `--name "John Doe"` |
| `int` | Integer values | `--count 42` |
| `double` | Floating-point values | `--ratio 3.14` |
| `bool` | Boolean values | `--enabled true` |

## Exception Handling

The library provides several exception types for different error conditions:

- `ArgumentError` - Base class for argument-related errors
- `ParseError` - Parsing failures
- `ValidationError` - Validation failures
- `MissingArgumentError` - Required arguments not provided
- `UnknownArgumentError` - Unrecognized arguments

## Usage Examples

### Command Line Formats

```bash
# Flags
./myapp --verbose -d

# Options with values
./myapp --output result.txt -p 8080
./myapp --config=/path/to/config.json

# Positional arguments
./myapp input.txt
./myapp input.txt output.txt

# Combined
./myapp --verbose --port=8080 input.txt
```

### Help Output

The library automatically generates formatted help messages:

```
A sample application

Usage: myapp [OPTIONS] input

Positional arguments:
  input        Input file (required)

Options:
  -v, --verbose    Enable verbose output
  -o, --output     Output file (default: output.txt)
  -h, --help       Show this help message

Version: 1.0.0
```
