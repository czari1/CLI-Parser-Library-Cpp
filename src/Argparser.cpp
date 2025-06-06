#include "ArgParser.hpp"

#include <iostream>
#include <algorithm>
#include <iomanip>

namespace argparser {

ArgParser::ArgParser(std::string_view programName, std::string_view description)
    : m_programName(programName), m_description(description) {}

Argument& ArgParser::addFlag(std::string_view shortName, std::string_view longName,
                    std::string_view description) {
    auto arg = std::make_unique<Argument> (shortName, longName, description);
    auto* argPtr = arg.get();

    m_arguments.push_back(std::move(arg));

    if(!shortName.empty()) {
        m_argMap[std::string(shortName)] = argPtr;
    }

    if(!longName.empty()) {
        m_argMap[std::string(longName)] = argPtr;
    }

    return *argPtr;
}

Argument& ArgParser::addOption(std::string_view shortName, std::string_view longName,
                    std::string_view description,
                    std::string_view defaultValue) {
    auto arg = std::make_unique<Argument> (shortName, longName, description, defaultValue);
    auto* argPtr = arg.get();

    m_arguments.push_back(std::move(arg));

    if(!shortName.empty()) {
        m_argMap[std::string(shortName)] = argPtr;
    }

    if(!longName.empty()) {
        m_argMap[std::string(longName)] = argPtr;
    }

    return *argPtr;
}

Argument& ArgParser::addPositional(std::string_view name,std::string_view description,
                                    bool required ) {
    auto arg = std::make_unique<Argument> (name, description, required);
    auto* argPtr = arg.get();

    m_arguments.push_back(std::move(arg));
    m_argMap[std::string(name)] = argPtr;

    return *argPtr;
}

void ArgParser::parseOptions(int argc, char* argv[]) {
    std::vector<std::string> args;
    args.reserve(argc);

    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    if (m_programName.empty() && argc > 0) {
        m_programName = argv[0];
    }

    parsePositionalOption(args);
}

void ArgParser::parsePositionalOption(const std::vector<std::string>& args) {
    auto it = args.begin();
    const auto end = args.end();

    while (it != end) {
        const std::string& arg = *it;

        if (arg == "--help" || arg == "-h") {
            printHelp();
            std::exit(0);
        }

        if (arg.starts_with("--")) {
            parseLongOption(arg, it, end);

        } else if (arg.starts_with("-") && arg.length() > 1) {
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
    std::string shortName = arg.substr(1,1);
    auto* argument = findArgument(shortName);

    if (!argument) {
        throw UnknownArgumentError(arg);
    }

    if (argument->type() == ArgumentType::Flag) {
        argument->setFlag(true);
        ++it;
    } else {

        if (arg.length() > 2) {
            argument->setValue(arg.substr(2));
        } else {
            ++it;
            if (it == end) {
                throw ParseError("Missing value for option: " + arg);
            }
            argument->setValue(*it);
        }
        ++it;
    }
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

    if (!argument) {
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
    auto* arg = findArgument(name);
    
    return arg ? arg->getString() : "";
}

int ArgParser::getInt(std::string_view name) const {
    auto* arg = findArgument(name);

    if (!arg) {
        throw ArgumentError("Argument not found: " + std::string(name));
    }

    return arg->getInt();
}

double ArgParser::getDouble(std::string_view name) const {
    auto* arg = findArgument(name);

    if (!arg) {
        throw ArgumentError("Argument not found: " + std::string(name));
    }

    return arg->getDouble();
}

bool ArgParser::getBool(std::string_view name) const {
    auto* arg = findArgument(name);

    return arg ? arg->isSet() : false;
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

Argument* ArgParser::findArgument(std::string_view name) const {
    auto it = m_argMap.find(std::string(name));

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
                oss << "}";
            }
        }
    }

    return oss.str();
}

std::string ArgParser::formatArguments() const {
    std::ostringstream oss;

    std::size_t maxWidth = 0;

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

            if (!hasOptions) {

                if( hasPositional) {
                    oss << "\n";
                }
                oss << "Options: \n";
                hasOptions = true;
            }

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

    return oss.str();
}

}