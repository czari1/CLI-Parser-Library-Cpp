#include "ArgParser.hpp"

#include <iostream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iomanip>

namespace argparser {

namespace {

std::string normalizeName(std::string_view name) {
    while (!name.empty() && name.front() == '-') {
        name.remove_prefix(1);
    }

    return std::string(name);
}

bool isNumericLiteral(std::string_view arg) {
    if (arg.size() < 2 || arg.front() != '-') {
        return false;
    }

    bool hasDigit = false;
    bool hasDot = false;

    for (std::size_t i = 1; i < arg.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(arg[i]);

        if (std::isdigit(c)) {
            hasDigit = true;
        } else if (arg[i] == '.' && !hasDot) {
            hasDot = true;
        } else {
            return false;
        }
    }

    return hasDigit;
}

void validateOptionNames(std::string_view shortName, std::string_view longName) {
    if (shortName.empty() && longName.empty()) {
        throw ArgumentError("Argument must have a short or long name");
    }

    if (shortName.size() > 1) {
        throw ArgumentError("Short name must be a single character: " + std::string(shortName));
    }
}

void ensureNameAvailable(const std::unordered_map<std::string, Argument*>& names,
                        const std::string& name) {
    if (!name.empty() && names.contains(name)) {
        throw ArgumentError("Duplicate argument name: " + name);
    }
}

}

ArgParser::ArgParser(std::string_view programName, std::string_view description)
    : m_programName(programName), m_description(description) {}

Argument& ArgParser::addFlag(std::string_view shortName, std::string_view longName,
                    std::string_view description) {
    const std::string normalizedShort = normalizeName(shortName);
    const std::string normalizedLong = normalizeName(longName);
    validateOptionNames(normalizedShort, normalizedLong);

    if (!normalizedShort.empty() && normalizedShort == normalizedLong) {
        throw ArgumentError("Duplicate argument name: " + normalizedShort);
    }

    ensureNameAvailable(m_argMap, normalizedShort);
    ensureNameAvailable(m_argMap, normalizedLong);

    auto arg = std::make_unique<Argument>(normalizedShort, normalizedLong, description);
    auto* argPtr = arg.get();

    m_arguments.push_back(std::move(arg));
    registerName(normalizedShort, argPtr);
    registerName(normalizedLong, argPtr);

    return *argPtr;
}

Argument& ArgParser::addOption(std::string_view shortName, std::string_view longName,
                    std::string_view description,
                    std::string_view defaultValue) {
    const std::string normalizedShort = normalizeName(shortName);
    const std::string normalizedLong = normalizeName(longName);
    validateOptionNames(normalizedShort, normalizedLong);

    if (!normalizedShort.empty() && normalizedShort == normalizedLong) {
        throw ArgumentError("Duplicate argument name: " + normalizedShort);
    }

    ensureNameAvailable(m_argMap, normalizedShort);
    ensureNameAvailable(m_argMap, normalizedLong);

    auto arg = std::make_unique<Argument>(normalizedShort, normalizedLong, description, defaultValue);
    auto* argPtr = arg.get();

    m_arguments.push_back(std::move(arg));
    registerName(normalizedShort, argPtr);
    registerName(normalizedLong, argPtr);

    return *argPtr;
}

Argument& ArgParser::addPositional(std::string_view name, std::string_view description,
                                    bool required) {
    const std::string normalizedName = normalizeName(name);

    if (normalizedName.empty()) {
        throw ArgumentError("Positional argument name cannot be empty");
    }

    if (required) {
        for (const auto& existing : m_arguments) {
            if (existing->type() == ArgumentType::Positional && !existing->isRequired()) {
                throw ArgumentError("Required positional argument cannot follow an optional one");
            }
        }
    }

    ensureNameAvailable(m_argMap, normalizedName);

    auto arg = std::make_unique<Argument>(normalizedName, description, required);
    auto* argPtr = arg.get();

    m_arguments.push_back(std::move(arg));
    registerName(normalizedName, argPtr);

    return *argPtr;
}

void ArgParser::parseOptions(int argc, char* argv[]) {
    std::vector<std::string> args;
    args.reserve(static_cast<std::size_t>(std::max(argc - 1, 0)));

    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    if (m_programName.empty() && argc > 0 && argv[0] != nullptr) {
        m_programName = argv[0];
    }

    parsePositionalOption(args);
}

void ArgParser::parsePositionalOption(const std::vector<std::string>& args) {
    m_positionalValues.clear();

    for (auto& argument : m_arguments) {
        argument->reset();
    }

    auto it = args.begin();
    const auto end = args.end();

    while (it != end) {
        const std::string& arg = *it;

        if (arg == "--") {
            ++it;

            for (; it != end; ++it) {
                m_positionalValues.push_back(*it);
            }
            break;
        }

        if ((arg == "--help" && findArgument("help") == nullptr) ||
            (arg == "-h" && findArgument("h") == nullptr)) {
            printHelp();
            std::exit(EXIT_SUCCESS);
        }

        if (arg.starts_with("--")) {
            parseLongOption(arg, it, end);

        } else if (arg.starts_with("-") && arg.length() > 1 && !isNumericLiteral(arg)) {
            parseShortOption(arg, it, end);

        } else {
            m_positionalValues.push_back(arg);
            ++it;
        }
    }

    std::size_t positionalIndex = 0;

    for (const auto& arg : m_arguments) {

        if (arg->type() == ArgumentType::Positional && positionalIndex < m_positionalValues.size()) {
            arg->setValue(m_positionalValues[positionalIndex++]);
        }
    }

    validateRequiredArgument();
}

void ArgParser::parseShortOption(const std::string& arg, std::vector<std::string>::const_iterator& it,
                        const std::vector<std::string>::const_iterator& end) {
    std::size_t index = 1;

    while (index < arg.size()) {
        const std::string shortName(1, arg[index]);
        auto* argument = findArgument(shortName);

        if (!argument || argument->type() == ArgumentType::Positional) {
            throw UnknownArgumentError(std::string("-") + shortName);
        }

        if (argument->type() == ArgumentType::Flag) {
            argument->setFlag(true);
            ++index;
            continue;
        }

        std::string value;

        if (index + 1 < arg.size()) {
            value = arg.substr(index + 1);

            if (!value.empty() && value.front() == '=') {
                value.erase(value.begin());
            }
        } else {
            ++it;

            if (it == end) {
                throw ParseError("Missing value for option: -" + shortName);
            }

            value = *it;
        }

        argument->setValue(value);
        ++it;
        return;
    }

    ++it;
}

void ArgParser::parseLongOption(const std::string& arg, std::vector<std::string>::const_iterator& it,
                               const std::vector<std::string>::const_iterator& end) {
    auto eqPos = arg.find('=');
    std::string longName;
    std::string value;

    if (eqPos != std::string::npos) {
        longName = arg.substr(2, eqPos - 2);
        value = arg.substr(eqPos + 1);
    } else {
        longName = arg.substr(2);
    }

    auto* argument = findArgument(longName);

    if (!argument || argument->type() == ArgumentType::Positional) {
        throw UnknownArgumentError(arg);
    }
    
    if ( argument->type() == ArgumentType::Flag) {

        if (eqPos != std::string::npos) {
            throw ParseError("Flag argument cannot have a value: " + arg);
        }
        argument->setFlag(true);
        ++it;
   } else {
        
        if (eqPos != std::string::npos) {
            
            argument->setValue(value);
            ++it;
        } else {
            ++it;
            
            if (it == end) {
                throw ParseError("Missing value for option: " + arg);
            }
            argument->setValue(*it);
            ++it;
        }
    }
}

void ArgParser::validateRequiredArgument() const {

    for (const auto& arg : m_arguments) {

        if (arg->isRequired() && !arg->isSet()) {
            
            if (arg->type() == ArgumentType::Positional) {
                
                throw MissingArgumentError(arg->name());
            } else {
                std::string name = !arg-> longName().empty() ?
                "--" + arg->longName() : "-" + arg->shortName();
                
                throw MissingArgumentError(name); 
            }
        } 
    }
}

std::string ArgParser::getString(std::string_view name) const {
    return requireArgument(name).getString();
}

int ArgParser::getInt(std::string_view name) const {
    return requireArgument(name).getInt();
}

double ArgParser::getDouble(std::string_view name) const {
    return requireArgument(name).getDouble();
}

bool ArgParser::getBool(std::string_view name) const {
    return requireArgument(name).getBool();
}

bool ArgParser::isSet(std::string_view name) const {
    auto* arg = findArgument(name);

    return arg ? arg->isSet() : false;
}

const std::vector<std::string>& ArgParser::positionalArguments() const noexcept {

    return m_positionalValues;
}

std::string ArgParser::help() const {
    std::ostringstream oss;

    if (!m_description.empty()) {
        oss << m_description << "\n\n";
    }

    oss << formatUsage() << "\n\n";
    oss << formatArguments();

    if (!m_version.empty()) {
        oss << "\nVersion: " << m_version;
    }

    return oss.str();
}

void ArgParser::printHelp() const {
    std::cout << help() << std::endl;
}

ArgParser& ArgParser::programName(std::string_view name) {
    m_programName = name;

    return *this;
} 

ArgParser& ArgParser::description(std::string_view desc) {
    m_description = desc;

    return *this;
}

ArgParser& ArgParser::version(std::string_view version) {
    m_version = version;

    return *this;
}

void ArgParser::registerName(const std::string& name, Argument* argument) {
    if (name.empty()) {
        return;
    }

    if (m_argMap.contains(name)) {
        throw ArgumentError("Duplicate argument name: " + name);
    }

    m_argMap.emplace(name, argument);
}

const Argument& ArgParser::requireArgument(std::string_view name) const {
    auto* arg = findArgument(name);

    if (!arg) {
        throw ArgumentError("Argument not found: " + std::string(name));
    }

    return *arg;
}

Argument* ArgParser::findArgument(std::string_view name) const {
    auto it = m_argMap.find(normalizeName(name));

    return it != m_argMap.end() ? it->second : nullptr;
}

std::string ArgParser::formatUsage() const {
    std::ostringstream oss;
    oss << "Usage: " << m_programName;

    bool hasOptions = false;

    for (const auto& arg : m_arguments) {

        if (arg->type() != ArgumentType::Positional) {

            if (!hasOptions) {
                oss << " [OPTIONS]";
                hasOptions = true;

                break;
            }
        }
    }

    for (const auto& arg : m_arguments) {

        if (arg->type() == ArgumentType::Positional) {
            oss << " ";

            if (!arg->isRequired()) {
                oss << "[";
            }
            oss << arg->name();

            if (!arg->isRequired()) {
                oss << "]";
            }
        }
    }

    return oss.str();
}

std::string ArgParser::formatArguments() const {
    std::ostringstream oss;

    std::size_t maxWidth = 0;
    const bool autoShortHelp = findArgument("h") == nullptr;
    const bool autoLongHelp = findArgument("help") == nullptr;
    std::string autoHelpLabel;

    if (autoShortHelp && autoLongHelp) {
        autoHelpLabel = "-h, --help";
    } else if (autoShortHelp) {
        autoHelpLabel = "-h";
    } else if (autoLongHelp) {
        autoHelpLabel = "--help";
    }

    if (!autoHelpLabel.empty()) {
        maxWidth = autoHelpLabel.size();
    }

    for (const auto& arg : m_arguments) {
        std::string argStr;

        if (arg->type() == ArgumentType::Positional) {
            argStr = arg->name();
        } else {

            if(!arg->shortName().empty()) {
                argStr += "-" + arg->shortName();
            }

            if (!arg-> longName().empty()) {

                if(!argStr.empty()){
                    argStr += ", ";
                }
                argStr += "--" + arg->longName();
            }
        }
        maxWidth = std::max(maxWidth, argStr.length());
    }

    bool hasPositional = false;
    bool hasOptions = false;

    auto ensureOptionsHeader = [&]() {
        if (!hasOptions) {

            if (hasPositional) {
                oss << "\n";
            }
            oss << "Options: \n";
            hasOptions = true;
        }
    };

    for (const auto& arg : m_arguments) {

        if (arg->type() == ArgumentType::Positional) {

            if (!hasPositional) {
                oss << "Positional arguments: \n";
                hasPositional = true;
            }
            oss << " " << std::left << std::setw(maxWidth) << arg->name()
                << " " << arg->description();
            
            if (arg->isRequired()) {
                oss << " (required)";
            }
            oss << "\n";
        }
    }

    for (const auto& arg : m_arguments) {

        if (arg->type() != ArgumentType::Positional) {
            ensureOptionsHeader();

            std::string argStr;

            if (!arg->shortName().empty()) {
                argStr += "-" + arg->shortName();
            }

            if (!arg-> longName().empty()) {

                if(!argStr.empty()){
                    argStr += ", ";
                }
                argStr += "--" + arg->longName();
            }

            oss << " " << std::left << std::setw(maxWidth) << argStr
                << " " << arg->description();

            if (!arg->defaultValue().empty()) {
                oss << " (default: " << arg-> defaultValue() << ")";
            }

            if (arg->isRequired()) {
                oss << " (required)";
            }
            oss << "\n";
        }
    }

    if (!autoHelpLabel.empty()) {
        ensureOptionsHeader();
        oss << " " << std::left << std::setw(static_cast<int>(maxWidth)) << autoHelpLabel
            << " Show this help message\n";
    }

    return oss.str();
}

}