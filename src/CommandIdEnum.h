#pragma once
#include "Arduino.h"

typedef enum : uint8_t {
    // 确保枚举占用一个字节
#define X(name) name,
    COMMAND_ID_LIST
#undef X
} CommandIdEnum;


inline const char *commandIdToStrPtr(const CommandIdEnum id) {
    static const char *commandStrs[] = {
#define X(name) #name,
        COMMAND_ID_LIST
#undef X
    };
    return commandStrs[id];
}

inline String commandIdToStr(const CommandIdEnum id) {
    return commandIdToStrPtr(id);
}

inline CommandIdEnum strToCommandId(const String &commandStr) {
#define X(name) if (commandStr == F(#name)) return name;
    COMMAND_ID_LIST
#undef X
#ifdef LOG_DEBUG
    Serial.print(F("strToCommandId, can not mapping CommandIdEnum: "));
    Serial.print(commandStr);
    Serial.println();
#endif
    return static_cast<CommandIdEnum>(0);
}

inline CommandIdEnum strToCommandId(const char *commandStr) {
#define X(name) if (strcmp(commandStr, #name) == 0) return name;
    COMMAND_ID_LIST
#undef X
#ifdef LOG_DEBUG
    Serial.print(F("strToCommandId, can not mapping commandStr: "));
    Serial.print(commandStr);
    Serial.println();
#endif
    return static_cast<CommandIdEnum>(0);
}

inline uint8_t commandIdEnumCount() {
    // 使用和字符串表同一套宏展开，保证数量永远一致
    static const char *dummy[] = {
#define X(name) #name,
        COMMAND_ID_LIST
#undef X
    };

    return static_cast<uint8_t>(sizeof(dummy) / sizeof(dummy[0]));
}
