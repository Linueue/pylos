#include <iostream>
#include <iomanip>

#include "lexer.hpp"

int main() {
    Lexer lexer(R"(val hasGuest: Bool = false

    // Ignore this comment
    // This is a test
    /*
        This is also a test
    */

    if hasGuest && 12 > 5 || 23 * 5 >= 5 {
        port Gi0/2 {
            mode = Access(vlan)
            enabled = true
            label = "Guest"
        }
    })");

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
