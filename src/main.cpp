#include <iostream>
#include <iomanip>

#include "lexer.hpp"

int main() {
    Lexer lexer("val x = 192.167.0.5/23 + 23 + 5 - 10 / 23 + 533");

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
