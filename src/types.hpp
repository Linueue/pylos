#pragma once

#include <cstdint>
#include <string>
#include <cassert>

struct IPv4 {
    constexpr explicit IPv4(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4)
        : m_Value(
            ((uint32_t)(o1) << 24) |
            ((uint32_t)(o2) << 16) |
            ((uint32_t)(o3) << 8 ) |
            ((uint32_t)(o4))
        ) {}

    constexpr uint32_t AsUint() const noexcept { return m_Value; }
    constexpr uint8_t GetOctet1() const noexcept {
        return m_Value >> 24 & 0xFF;
    }
    constexpr uint8_t GetOctet2() const noexcept {
        return m_Value >> 16 & 0xFF;
    }
    constexpr uint8_t GetOctet3() const noexcept {
        return m_Value >> 8 & 0xFF;
    }
    constexpr uint8_t GetOctet4() const noexcept {
        return m_Value & 0xFF;
    }

    std::string AsString() const {
        std::string buf = std::to_string((int32_t)GetOctet1());
        buf += std::string(".") + std::to_string((int32_t)GetOctet2());
        buf += std::string(".") + std::to_string((int32_t)GetOctet3());
        buf += std::string(".") + std::to_string((int32_t)GetOctet4());
        return buf;
    }

private:
    uint32_t m_Value;
};

struct Cidr {
    IPv4 ip;
    uint8_t cidr;
};
