#pragma once
#include <map>
#include "../CommandContext.h"
#include "../CommandManager.h"
#include "../RtosHelper.h"

class IRTransport : public CommandManager<CommandId_Type, CommandData_Type> {
protected:
    const int receivePin;
    const std::map<long, CommandId_Type> commandMapping;
    TaskWrapper *p_taskWrapper = nullptr;

public:
    IRTransport(int receivePin, const std::map<long, CommandId_Type> &commandMapping);

    void setup();

    void receiveData() const;
    void consumeIrMessage(long message) const;
};
