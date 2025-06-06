#pragma once

#include <stdexcept>
#include <string>

namespace argparser {

class ArgumentError : public std::runtime_error{
public:
    explicit ArgumentError(const std::string &message)
        : std::runtime_error("Argument Error: " + message) {}
};

class ParseError : public std::runtime_error{
public:
    explicit ParseError(const std::string &message)
        : std::runtime_error("Parse Error: " + message) {}
};

class ValidationError : public std::runtime_error{
public:
    explicit ValidationError(const std::string &message)
        : std::runtime_error("Validation Error: " + message) {}
};

class MissingArgumentError : public ArgumentError{
public:
    explicit MissingArgumentError(const std::string &message)
        : ArgumentError("Missing required argument: " + message) {}
};

class UnknownArgumentError : public ArgumentError{
public:
    explicit UnknownArgumentError(const std::string &message)
        : ArgumentError("Unknown argument: " + message) {}
};

}
