#pragma once
#include "../CommandContext.h"
#include "../CommandManager.h"


template<typename CommandId_Type, typename CommandData_Type>
class BaseUartTransport : public CommandManager<CommandId_Type, CommandData_Type> {
public:
    BaseUartTransport(int tx, int rx);

    void setup();
};
