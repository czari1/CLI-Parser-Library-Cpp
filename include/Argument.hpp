#pragma once 

#include <string>
#include <string_view>
#include <optional>
#include <variant>
#include <vector>
#include <functional>
#include <algorithm>
#include <cctype>


namespace argparser {
    
enum class ArgumentType {
    Flag,
    Option,
    Positional
};

class Argument {
public:
    using ValueType = std::variant<std::string, int, double, bool>;
    using ValidatorFunction = std::function<bool(const std::string&)>;

    Argument(std::string_view shortName, std::string_view longName, 
            std::string_view description);
    
    Argument(std::string_view shortName, std::string_view longName, 
            std::string_view description, std::string_view defaultValue);
    
    explicit Argument(std::string_view name, std::string_view description,
                        bool required = false);
    
    Argument& required(bool isRequired = true);
    Argument& defaultValue(std::string_view value);
    Argument& help(std::string_view description);
    Argument& validator(ValidatorFunction func);
    
    template<typename T>
    Argument& defaultValue(T value);

    [[nodiscard]] const std::string& shortName() const noexcept { return m_shortName; }
    [[nodiscard]] const std::string& longName() const noexcept { return m_longName; }
    [[nodiscard]] const std::string& name() const noexcept { return m_name; }
    [[nodiscard]] const std::string& description() const noexcept { return m_description; }
    [[nodiscard]] const std::string& defaultValue() const noexcept { return m_defaultValue; }
    [[nodiscard]] ArgumentType type() const noexcept { return m_type; }
    [[nodiscard]] bool isRequired() const noexcept { return m_isRequired; }
    [[nodiscard]] bool isSet() const noexcept { return m_isSet; }

    void setValue(std::string_view value);
    void setFlag(bool value = true);
    
    template<typename T>
    [[nodiscard]] std::optional<T> get() const;
    
    [[nodiscard]] std::string getString() const;
    [[nodiscard]] int getInt() const;
    [[nodiscard]] double getDouble() const;
    [[nodiscard]] bool getBool() const;
    
    [[nodiscard]] bool validate(const std::string& value) const;

private:
    std::string m_shortName;
    std::string m_longName;
    std::string m_name;
    std::string m_description;
    std::string m_defaultValue;
    std::string m_currentValue;
    ArgumentType m_type;
    bool m_isRequired = false;
    bool m_isSet = false;
    ValidatorFunction m_validator; 
};

template<typename T>
Argument& Argument::defaultValue(T value) {
    
    if constexpr (std::is_same_v<T, std::string>) {
        m_defaultValue = value;
    
    } else if constexpr (std::is_arithmetic_v<T>) {
        m_defaultValue = std::to_string(value);
    
    } else {
        static_assert(sizeof(T) == 0, "Unsupported type for default value");
    }
    
    return *this;
}

template<typename T>
std::optional<T> Argument::get() const {

    if(!m_isSet && m_defaultValue.empty()) {
        
        return std::nullopt;
    }

    const std::string& value = m_isSet ? m_currentValue : m_defaultValue;

    if constexpr (std::is_same_v<T, std::string>) {

        return value;
    
    } else if constexpr (std::is_same_v<T, int>) {

        try {

            return std::stoi(value);

        } catch (...) {

            return std::nullopt;
        }
    
    } else if constexpr (std::is_same_v<T, double>) {

        try {

            return std::stod(value);
        
        } catch (...) {

            return std::nullopt;
        }
    
    } else if constexpr (std::is_same_v<T, bool>) { 

        if (m_type == ArgumentType::Flag) {

            return m_isSet;
        }

        std::string lowerValue = value;
        std::transform(lowerValue.begin(), lowerValue.end(),
                        lowerValue.begin(), [](char c) 
                        { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });

        return (lowerValue == "true" || lowerValue == "1"||
                lowerValue == "yes" || lowerValue == "on");
    
    } else {
        static_assert(sizeof(T) == 0, "Unsupported type for get()");
    }
}

}
