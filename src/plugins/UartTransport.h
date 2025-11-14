#pragma once
#include "../CommandContext.h"
#include "../CommandManager.h"


class BaseUartTransport : public CommandManager<CommandId_Type, CommandData_Type> {
public:
    BaseUartTransport(int tx, int rx);

    void setup();
};
