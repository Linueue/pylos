#include "token.hpp"

#include <string>

#define X(name) #name
static constexpr const std::string_view g_TokenStrings[] = {
    TOKEN_LIST()
};
#undef X

Expected<int32_t, const char*> Token::AsInt() const {
    if(type != TokenType::IntegerLiteral)
        return Unexpected("Invalid conversion");

    return std::stoi(std::string(lexeme));
}

static Expected<IPv4, const char*> asIPv4(const std::string_view lexeme) {
    uint8_t octets[4];
    int32_t currentOctet = 0;
    int32_t idx = 0;

    for(size_t i = 0; i < lexeme.size() + 1; i++) {
        if(lexeme[i] == '.' || i == lexeme.size()) {
            const std::string octet(lexeme.substr(idx, i - idx));
            const int32_t converted = std::stoi(octet);

            if(converted > 255)
                return Unexpected("Invalid IPv4, octet must range: 0..255");

            octets[currentOctet++] = converted;
            idx = i + 1;
            continue;
        }
    }

    return IPv4(octets[0], octets[1], octets[2], octets[3]);
}

Expected<IPv4, const char*> Token::AsIPv4() const {
    if(type != TokenType::IPv4Literal)
        return Unexpected("Invalid conversion");

    return asIPv4(lexeme);
}

Expected<Cidr, const char*> Token::AsCIDR() const {
    if(type != TokenType::IPv4CIDRLiteral)
        return Unexpected("Invalid conversion");

    const size_t slashIdx = lexeme.find('/');
    auto ip = asIPv4(lexeme.substr(0, slashIdx));

    if(ip.IsError())
        return Unexpected(ip.Error());

    std::string cidrString(lexeme.substr(slashIdx + 1, lexeme.size()));
    int32_t cidr = std::stoi(cidrString);

    if(cidr > 32)
        return Unexpected("CIDR must range: 0..32");

    return Cidr { .ip = *ip, .cidr = (uint8_t)cidr };
}

std::string_view Token::GetTokenString() const {
    return g_TokenStrings[(int32_t)type];
}

std::string_view Token::GetTokenString(TokenType type) {
    return g_TokenStrings[(int32_t)type];
}
