#include "token.hpp"

#include <string>

#define X(name) #name
static constexpr const std::string_view g_TokenStrings[] = {
    TOKEN_LIST()
};
#undef X

int32_t Token::AsInt() const {
    return std::stoi(std::string(lexeme));
}

static IPv4 asIPv4(const std::string_view lexeme) {
    uint8_t octets[4];
    int32_t currentOctet = 0;
    int32_t idx = 0;

    for(int32_t i = 0; i < lexeme.size(); i++) {
        if(lexeme[i] == '.') {
            const std::string octet(lexeme.substr(idx, i + idx));
            octets[currentOctet++] = std::stoi(octet);

            idx = i + 1;
            continue;
        }
    }
    const std::string octet(lexeme.substr(idx, lexeme.size()));
    octets[currentOctet++] = std::stoi(octet);

    return IPv4(octets[0], octets[1], octets[2], octets[3]);
}

IPv4 Token::AsIPv4() const {
    return asIPv4(lexeme);
}

Cidr Token::AsCIDR() const {
    const size_t slashIdx = lexeme.find('/');
    IPv4 ip = asIPv4(lexeme.substr(0, slashIdx));
    std::string cidrString(lexeme.substr(slashIdx + 1, lexeme.size()));
    uint8_t cidr = std::stoi(cidrString);
    return Cidr { .ip = ip, .cidr = cidr };
}

std::string_view Token::GetTokenString() const {
    return g_TokenStrings[(int32_t)type];
}
