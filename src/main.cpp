#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

#include "lexer.hpp"

struct Args {
    std::string_view ProgramFile;
};

static Expected<Args, const char*> handleArgs(const std::vector<std::string_view> args) {
    Args argOut;

    if(args.size() != 2)
        return Unexpected("Invalid number of arguments.\nExpected: Pylos.exe <filepath>");

    argOut.ProgramFile = args[1];

    return argOut;
}

static const Expected<std::string, const char*> readFile(const std::string_view path) {
    std::ifstream file(path.data());

    if(!file)
        return Unexpected("Could not open file.");

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string contents = buffer.str();
    return contents;
}

int main(int32_t argc, char* argv[]) {
    const std::vector<std::string_view> argsList(argv, argv + argc);
    auto args = handleArgs(argsList);

    if(args.IsError()) {
        std::cerr << args.Error() << "\n";
        return -1;
    }

    auto contents = readFile((*args).ProgramFile);

    if(contents.IsError()) {
        std::cerr << args.Error() << "\n";
        return -1;
    }

    Lexer lexer(*contents);

    while(true) {
        auto tok = lexer.Next();

        if(tok.IsError()) {
            std::cout << "Error: " << tok.Error().reason << " at (" << tok.Error().line << ", " << tok.Error().column << ")\n";
            break;
        }

        std::cout << "[" << std::setw(10) << (*tok).GetTokenString() << "] " << (*tok).lexeme << "\n";

        if((*tok) == TokenType::EndOfFile)
            break;
    }
}
