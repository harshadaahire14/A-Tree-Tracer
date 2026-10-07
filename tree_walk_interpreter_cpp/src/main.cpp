#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static void run(const std::string& source, bool trace) {
    try {
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();

        Parser parser(tokens);
        auto statements = parser.parse();

        Interpreter interpreter;
        interpreter.setTracing(trace);
        interpreter.interpret(statements);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }
}

static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Could not open file: " + path);

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static void repl() {
    std::cout << "Tree-Walk Interpreter REPL\n";
    std::cout << "Type expressions/statements ending with ';'. Type 'exit' to quit.\n";
    std::cout << "Tracing can be enabled with: :trace on / :trace off\n\n";

    bool trace = false;
    std::string line;

    while (true) {
        std::cout << ">>> ";
        if (!std::getline(std::cin, line)) break;

        if (line == "exit") break;
        if (line == ":trace on") {
            trace = true;
            std::cout << "Tracing enabled.\n";
            continue;
        }
        if (line == ":trace off") {
            trace = false;
            std::cout << "Tracing disabled.\n";
            continue;
        }

        run(line, trace);
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        repl();
        return 0;
    }

    bool trace = false;
    std::string file;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--trace") {
            trace = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout
                << "Tree-Walk Interpreter\n\n"
                << "Usage:\n"
                << "  treewalk                 Start REPL\n"
                << "  treewalk file.tw         Run a source file\n"
                << "  treewalk --trace file.tw Run with AST execution tracing\n";
            return 0;
        } else if (file.empty()) {
            file = arg;
        } else {
            std::cerr << "Unknown argument: " << arg << '\n';
            return 1;
        }
    }

    if (file.empty()) {
        std::cerr << "No source file supplied.\n";
        return 1;
    }

    try {
        run(readFile(file), trace);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}
