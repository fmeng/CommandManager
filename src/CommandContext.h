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
struct CommandData;

using DataAccesser = std::function<void(CommandData &data)>;

struct CommandData {
    int id = 0;

    void accessMutex(const DataAccesser &dataAccesser);

    void dump(CommandData *dst);

    CommandData &dump();

    static CommandData *instance();

    CommandData(const CommandData &) = delete;

    CommandData &operator=(const CommandData &) = delete;

private:
    CommandData() = default;
};

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
