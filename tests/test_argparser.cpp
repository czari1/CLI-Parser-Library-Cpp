#include <gtest/gtest.h>
#include "ArgParser.hpp"

using argparser::ArgParser;
using argparser::HelpRequested;
using argparser::MissingArgumentError;
using argparser::ValidationError;

static std::vector<char*> makeArgv(const std::vector<std::string>& args) {
    static std::vector<std::string> storage;
    static std::vector<char*> argv;
    storage = args;
    argv.clear();
    for (auto& s : storage) {
        argv.push_back(s.data());
    }
    return argv;
}

TEST(ArgParser, ShortAndLongFlags) {
    ArgParser parser("app", "demo");
    parser.addFlag("v", "verbose", "Verbose");
    auto argv = makeArgv({"app", "-v", "--verbose"});
    parser.parseOptions(static_cast<int>(argv.size()), argv.data());
    EXPECT_TRUE(parser.getBool("verbose"));
}

TEST(ArgParser, LongOptionEqualsSyntax) {
    ArgParser parser("app", "demo");
    parser.addOption("", "name", "Name", "default");
    auto argv = makeArgv({"app", "--name=alice"});
    parser.parseOptions(static_cast<int>(argv.size()), argv.data());
    EXPECT_EQ(parser.getString("name"), "alice");
}

TEST(ArgParser, MissingRequiredPositional) {
    ArgParser parser("app", "demo");
    parser.addPositional("input", "Input file", true);
    auto argv = makeArgv({"app"});
    EXPECT_THROW(parser.parseOptions(static_cast<int>(argv.size()), argv.data()),
                 MissingArgumentError);
}

TEST(ArgParser, ValidatorRejectsValue) {
    ArgParser parser("app", "demo");
    auto& port = parser.addOption("p", "port", "Port", "8080");
    port.validator([](const std::string& value) {
        try {
            const int p = std::stoi(value);
            return p > 0 && p < 65536;
        } catch (...) {
            return false;
        }
    });
    auto argv = makeArgv({"app", "--port", "99999"});
    EXPECT_THROW(parser.parseOptions(static_cast<int>(argv.size()), argv.data()),
                 ValidationError);
}

TEST(ArgParser, HelpDoesNotExitProcess) {
    ArgParser parser("app", "demo");
    auto argv = makeArgv({"app", "--help"});
    EXPECT_THROW(parser.parseOptions(static_cast<int>(argv.size()), argv.data()),
                 HelpRequested);
}
