#pragma once
#include <vector>
#include "CommandIdEnum.h"

inline uint32_t commandsToEventBits() {
    return 0u;
}

template<typename First, typename... Rest>
uint32_t commandsToEventBits(First first, Rest... rest) {
    return (1u << static_cast<uint8_t>(first)) | commandsToEventBits(rest...);
}

inline std::vector<CommandIdEnum> eventBitsToCommands(const uint32_t bits) {
    std::vector<CommandIdEnum> result;
    for (int i = 0; i < sizeof(bits) * 8; i++) {
        if (bits & (1u << i)) {
            result.push_back(static_cast<CommandIdEnum>(i));
        }
    }
    return result;
}
