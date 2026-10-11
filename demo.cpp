#include "ArgParser.hpp"
#include <iostream>
#include <string>

void printHeader(const std::string& title) {
    std::cout << "\n" << std::string(50, '-') << std::endl;
    std::cout << title << std::endl;
    std::cout << std::string(50, '-') << std::endl;
}

void demoBasicFlags() {
    printHeader("DEMO 1: Basic Flags");
    
    // Simulate: myapp --verbose --debug
    const char* argv[] = {"myapp", "--verbose", "--debug"};
    int argc = 3;
    
    std::cout << "Command: myapp --verbose --debug\n" << std::endl;
    
    argparser::ArgParser parser("myapp", "Simple application with flags");
    
    // Add some flags
    parser.addFlag("v", "verbose", "Show detailed output");
    parser.addFlag("d", "debug", "Enable debug mode");
    parser.addFlag("q", "quiet", "Suppress output");
    
    parser.parseOptions(argc, const_cast<char**>(argv));
    
    std::cout << "Results:" << std::endl;
    std::cout << "  verbose: " << (parser.getBool("verbose") ? "ON" : "OFF") << std::endl;
    std::cout << "  debug: " << (parser.getBool("debug") ? "ON" : "OFF") << std::endl;
    std::cout << "  quiet: " << (parser.getBool("quiet") ? "ON" : "OFF") << std::endl;
}

void demoShortFlags() {
    printHeader("DEMO 2: Short Flags");
    
    // Simulate: myapp -vq
    const char* argv[] = {"myapp", "-vq"};
    int argc = 2;

    std::cout << "Command: myapp -vq\n" << std::endl;
    
    argparser::ArgParser parser("myapp", "Using short flag names");
    
    parser.addFlag("v", "verbose", "Verbose output");
    parser.addFlag("q", "quiet", "Quiet mode");
    parser.addFlag("h", "help", "Show help");
    
    parser.parseOptions(argc, const_cast<char**>(argv));
    
    std::cout << "Results:" << std::endl;
    std::cout << "  -v (verbose): " << (parser.getBool("v") ? "ON" : "OFF") << std::endl;
    std::cout << "  -q (quiet): " << (parser.getBool("q") ? "ON" : "OFF") << std::endl;
    std::cout << "  -h (help): " << (parser.getBool("h") ? "ON" : "OFF") << std::endl;
}

void demoPositionalArgs() {
    printHeader("DEMO 3: Positional Arguments");
    
    // Simulate: myapp input.txt output.txt
    const char* argv[] = {"myapp", "input.txt", "output.txt"};
    int argc = 3;
    
    std::cout << "Command: myapp input.txt output.txt\n" << std::endl;
    
    argparser::ArgParser parser("myapp", "File processing with positional args");
    
    parser.addPositional("input", "Input file", true);   // required
    parser.addPositional("output", "Output file", false); // optional
    
    parser.parseOptions(argc, const_cast<char**>(argv));
    
    std::cout << "Results:" << std::endl;
    std::cout << "  input file: " << parser.getString("input") << std::endl;
    std::cout << "  output file: " << parser.getString("output") << std::endl;
}

void demoMixedArguments() {
    printHeader("DEMO 4: Mixed Arguments (Flags + Positional)");
    
    // Simulate: myapp --verbose input.txt --debug output.txt
    const char* argv[] = {"myapp", "--verbose", "input.txt", "--debug", "output.txt"};
    int argc = 5;
    
    std::cout << "Command: myapp --verbose input.txt --debug output.txt\n" << std::endl;
    
    argparser::ArgParser parser("myapp", "Mixed flags and positional arguments");
    
    // Add flags
    parser.addFlag("v", "verbose", "Verbose output");
    parser.addFlag("d", "debug", "Debug mode");
    
    // Add positional arguments
    parser.addPositional("input", "Input file", true);
    parser.addPositional("output", "Output file", false);
    
    parser.parseOptions(argc, const_cast<char**>(argv));
    
    std::cout << "Flags:" << std::endl;
    std::cout << "  verbose: " << (parser.getBool("verbose") ? "ON" : "OFF") << std::endl;
    std::cout << "  debug: " << (parser.getBool("debug") ? "ON" : "OFF") << std::endl;
    
    std::cout << "Files:" << std::endl;
    std::cout << "  input: " << parser.getString("input") << std::endl;
    std::cout << "  output: " << parser.getString("output") << std::endl;
}

void demoWithOptions() {
    printHeader("DEMO 5: Flags + Options + Positional");
    
    // Simulate: myapp --verbose --format=pdf --count 5 document.txt
    const char* argv[] = {"myapp", "--verbose", "--format=pdf", "--count", "5", "document.txt"};
    int argc = 6;
    
    std::cout << "Command: myapp --verbose --format=pdf --count 5 document.txt\n" << std::endl;
    
    argparser::ArgParser parser("myapp", "Complete example with all argument types");
    
    // Flags
    parser.addFlag("v", "verbose", "Verbose output");
    parser.addFlag("s", "silent", "Silent mode");
    
    // Options (arguments with values)
    parser.addOption("f", "format", "Output format", "txt");
    parser.addOption("c", "count", "Number of copies", "1");
    
    // Positional
    parser.addPositional("document", "Document to process", true);
    
    parser.parseOptions(argc, const_cast<char**>(argv));
    
    std::cout << "Flags:" << std::endl;
    std::cout << "  verbose: " << (parser.getBool("verbose") ? "ON" : "OFF") << std::endl;
    std::cout << "  silent: " << (parser.getBool("silent") ? "ON" : "OFF") << std::endl;
    
    std::cout << "Options:" << std::endl;
    std::cout << "  format: " << parser.getString("format") << std::endl;
    std::cout << "  count: " << parser.getString("count") << std::endl;
    
    std::cout << "Positional:" << std::endl;
    std::cout << "  document: " << parser.getString("document") << std::endl;
}

void demoHelpGeneration() {
    printHeader("DEMO 6: Auto-Generated Help");
    
    argparser::ArgParser parser("file-tool", "A simple file processing tool");
    parser.version("1.0");
    
    // Add various arguments
    parser.addFlag("v", "verbose", "Show detailed output");
    parser.addFlag("f", "force", "Force overwrite existing files");
    parser.addOption("o", "output", "Output directory", "./output");
    parser.addPositional("input", "Input file to process", true);
    parser.addPositional("backup", "Backup location", false);
    
    std::cout << "Generated help message:\n" << std::endl;
    std::cout << parser.help() << std::endl;
}

int main() {
    std::cout << "ArgParser Library - Simple Demonstration" << std::endl;
    std::cout << "Focusing on Flags and Positional Arguments" << std::endl;
    
    demoBasicFlags();
    demoShortFlags();
    demoPositionalArgs();
    demoMixedArguments();
    demoWithOptions();
    demoHelpGeneration();
    
    printHeader("DEMONSTRATION COMPLETE");
    std::cout << "Key Features Shown:" << std::endl;
    std::cout << "  * Flags (boolean arguments): --verbose, -v" << std::endl;
    std::cout << "  * Positional arguments: input.txt output.txt" << std::endl;
    std::cout << "  * Mixed usage: flags and positional together" << std::endl;
    std::cout << "  * Options with values: --format=pdf" << std::endl;
    std::cout << "  * Auto-generated help messages" << std::endl;
    std::cout << "  * Both short (-v) and long (--verbose) forms" << std::endl;
    
    return 0;
}