#include "Argument.hpp"
#include "Exceptions.hpp"

#include <algorithm>
#include <cctype>

namespace argparser {

Argument::Argument(std::string_view shortName, std::string_view longName,
                    std::string_view description)
    : m_shortName(shortName), m_longName(longName), m_description(description),
    m_type(ArgumentType::Flag) {}

Argument::Argument(std::string_view shortName, std::string_view longName, 
                    std::string_view description, std::string_view defaultValue)
    : m_shortName(shortName), m_longName(longName), m_description(description),
    m_defaultValue(defaultValue), m_type(ArgumentType::Option) {}

Argument::Argument(std::string_view name, std::string_view description,
                    bool required)
    : m_name(name), m_description(description), m_isRequired(required),
    m_type(ArgumentType::Positional) {}

Argument& Argument::required(bool isRequired) {
    m_isRequired = isRequired;

    return *this;
}

Argument& Argument::defaultValue(std::string_view value) {
    m_defaultValue = value;

    return *this;
}

Argument& Argument::help(std::string_view description) {
    m_description = description;

    return *this;
}

Argument& Argument::validator(ValidatorFunction func) {
    m_validator = std::move(func);

    return *this;
}

void Argument::setValue(std::string_view value) {

    if (m_type ==ArgumentType::Flag) {
        throw ValidationError("Cannot set value for flag argument");
    }

    std::string valueStr(value);

    if (m_validator && !m_validator(valueStr)) {
        throw ValidationError("Invalid value for argument:" + valueStr);
    }

    m_currentValue = valueStr;
    m_isSet = true;
}

void Argument::setFlag(bool value) {

    if (m_type != ArgumentType::Flag) {
        throw ValidationError("Cannot set flag for non-flag argument");
    }

    m_isSet = value;
}

std::string Argument::getString() const {
    auto result = get<std::string>();
    
    return result ? *result : "";
}

int Argument::getInt() const {
    auto result = get<int>();

    if (!result) {
        throw ValidationError ("Cannot convert value to int");
    }
    
    return *result;
}

double Argument::getDouble() const {
    auto result = get<double>();

    if (!result) {
        throw ValidationError ("Cannot convert value to double");
    }
    
    return *result;
}

bool Argument::getBool() const {
    auto result = get<bool>();
    
    return result ? *result : false;
}

bool Argument::validate (const std::string& value) const {
    
    return !m_validator || m_validator(value);
}

}