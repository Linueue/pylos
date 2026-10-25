#include <iostream>
#include <iomanip>

#include "lexer.hpp"

int main() {
    Lexer lexer(R"(network Campus {
        val hr: Subnet = 192.168.10.0/24
        val hrVlan = vlan(id = 10, subnet = hr, gateway = hr.host(1))
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
