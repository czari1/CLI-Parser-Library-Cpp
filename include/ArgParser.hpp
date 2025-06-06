#pragma once

#include "Argument.hpp"
#include "Exceptions.hpp"

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>
#include <optional>
#include <sstream>

namespace argparser {

class ArgParser {
public:
    
    explicit ArgParser(std::string_view programName = "",
                        std::string_view description = "");
    
    ~ArgParser() = default;

    ArgParser(const ArgParser&) = delete;
    ArgParser& operator=(const ArgParser&) = delete;
    ArgParser(ArgParser&&) = default;
    ArgParser& operator=(ArgParser&&) = default;

    Argument& addFlag(std::string_view shortName, std::string_view longName,
                    std::string_view description);
    
    Argument& addOption(std::string_view shortName, std::string_view longName,
                    std::string_view description,
                    std::string_view defaultValue = "");
    
    Argument& addPositional(std::string_view name, std::string_view description,
                            bool required = false);

    void parseOptions(int argc, char* argv[]);
    void parsePositionalOption(const std::vector<std::string>& args);

    template <typename T>
    [[nodiscard]] std::optional<T> get(std::string_view name) const;

    [[nodiscard]] std::string getString(std::string_view name) const;
    [[nodiscard]] int getInt(std::string_view name) const;
    [[nodiscard]] double getDouble (std::string_view name) const;
    [[nodiscard]] bool getBool (std::string_view name) const;

    [[nodiscard]] bool isSet(std::string_view name) const;

    [[nodiscard]] const std::vector<std::string>& positionalArguments() const noexcept;

    [[nodiscard]] std::string help() const;
    void printHelp() const;

    ArgParser& programName(std::string_view name);
    ArgParser& description(std::string_view desc);
    ArgParser& version(std::string_view version);

private:
    std::string m_programName;
    std::string m_description;
    std::string m_version;

    std::vector<std::unique_ptr<Argument>> m_arguments;
    std::unordered_map<std::string, Argument*> m_argMap;
    std::vector<std::string> m_positionalValues;

    /*void parseArgument(const std::string& arg, std::vector<std::string>::const_iterator& it,
                        const std::vector<std::string>::const_iterator& end);*/
    void parseShortOption(const std::string& arg, std::vector<std::string>::const_iterator& it,
                        const std::vector<std::string>::const_iterator& end);
    void parseLongOption(const std::string& arg);
    void validateRequiredArgument() const;

    [[nodiscard]] Argument* findArgument(std::string_view name) const;
    [[nodiscard]] std::string formatUsage() const;
    [[nodiscard]] std::string formatArguments() const;
};

template<typename T>
std::optional<T> ArgParser::get(std::string_view name) const {
    auto* arg = findArgument(name);

    if (!arg) {
        
        return std::nullopt;
    }
    
    return arg->get<T>();
} 

}
