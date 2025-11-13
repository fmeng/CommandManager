#pragma once

#include <Arduino.h>
#include <map>
#include <functional>

/******************************************** Command ID ********************************************/

enum class CommandIdEnum : uint8_t {
    COMMAND_ID_TURN_ON,
    COMMAND_ID_TURN_OFF,
    COMMAND_ID_TURN_LEFT,

    // 占位
    COMMAND_ID_HOLDER_MAX,
};

/******************************************** Command Data (Singleton) ********************************************/
#pragma pack(push, 1)  //// 确保结构体不会有额外填充, 开始 1 字节对齐

struct CommandData {
    int id = 0;

    static CommandData *instance();

    CommandData(const CommandData &) = delete;

    CommandData &operator=(const CommandData &) = delete;

private:
    CommandData() = default;
};

#pragma pack(pop)  //// 确保结构体不会有额外填充, 恢复默认对齐


/******************************************** Handler Types ********************************************/

using CommandId_Type = CommandIdEnum;
using CommandData_Type = CommandData;
using CommandHandler = std::function<void(CommandId_Type commandId, CommandData_Type &command_data, void *p_attach)>;

/******************************************** Global Registry API ********************************************/

// 声明：返回全局 handlers（定义在 cpp 中）
std::map<CommandId_Type, CommandHandler> &getGlobalCommandHandlers();

/******************************************** Auto Register System ********************************************/

struct AutoRegister {
    AutoRegister(CommandId_Type commandId, const CommandHandler &commandHandler);
};

// 宏：使用函数名做唯一静态变量名
#define REGISTER_HANDLER(id, fn) \
static AutoRegister _auto_register_##fn (id, fn)
