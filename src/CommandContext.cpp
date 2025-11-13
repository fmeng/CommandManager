#include "CommandContext.h"
#define LOG_DEBUG
/******************************************** CommandData Singleton ********************************************/

CommandData *CommandData::instance() {
    static CommandData instance;
    return &instance;
}

void CommandData::accessMutex(const DataAccesser &dataAccesser) {
    static SemaphoreHandle_t dataMutex = xSemaphoreCreateMutex();
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
        dataAccesser(*this);
        xSemaphoreGive(dataMutex);
    }
}


void  CommandData::dump(CommandData *dst) {
    DataAccesser dumper = [dst](const CommandData &data) {
        memcpy(dst, &data, sizeof(CommandData));
    };
    accessMutex(dumper);
}

CommandData &CommandData::dump() {
    static CommandData innerData;
    static CommandData *p_innerData = &innerData;
    static DataAccesser dumper = [](const CommandData &data) {
        memcpy(p_innerData, &data, sizeof(CommandData));
    };
    accessMutex(dumper);
    return innerData;
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
