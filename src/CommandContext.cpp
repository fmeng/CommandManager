#include "CommandContext.h"
#define LOG_DEBUG
/******************************************** CommandData Singleton ********************************************/

CommandData *CommandData::instance() {
    static CommandData instance;
    return &instance;
}

/******************************************** Global Handler Registry ********************************************/

std::map<CommandId_Type, CommandHandler> &getGlobalCommandHandlers() {
    static std::map<CommandId_Type, CommandHandler> global_commandHandlers{};
    return global_commandHandlers;
}

/******************************************** AutoRegister Implementation ********************************************/

AutoRegister::AutoRegister(const CommandId_Type commandId, const CommandHandler &commandHandler) {
    getGlobalCommandHandlers()[commandId] = commandHandler;
#ifdef LOG_DEBUG
    Serial.print("AutoRegister");
    Serial.print(", commandId: ");
    Serial.print(static_cast<int>(commandId));
    Serial.println();
#endif
}
