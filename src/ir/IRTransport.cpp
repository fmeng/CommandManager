#include "IRTransport.h"
#include <IRremote.h>
#include "../RtosHelper.h"

IRTransport::IRTransport(const int receivePin,
                         const std::map<long, CommandId_Type> &commandMapping) : receivePin(receivePin),
    commandMapping(commandMapping) {
}

void IRTransport::setup() {
    p_taskWrapper = new TaskWrapper([this]() {
                                        IrReceiver.begin(receivePin, DISABLE_LED_FEEDBACK);
                                    }, [this]() {
                                        this->receiveData();
                                    }, 50);
}

void IRTransport::consumeIrMessage(long message) const {
    if (commandMapping.find(message) == commandMapping.end()) {
#ifdef LOG_DEBUG
        Serial.print("IrTransport::consumeIrMessage");
        Serial.print(", no mapping, ");
        Serial.print(", message: ");
        Serial.print(message);
        Serial.println();
#endif
        return;
    }

    const CommandId_Type commandId = commandMapping.at(message);
    const bool res = handleCommand(commandId, *p_commandData, nullptr);

#ifdef LOG_DEBUG
    Serial.print("IrTransport::consumeIrMessage");
    Serial.print(", res: ");
    Serial.print(res);
    Serial.print(", message: ");
    Serial.print(message);
    Serial.print(", commandId: ");
    Serial.print(static_cast<int>(commandId));
    Serial.println();
#endif
}


void IRTransport::receiveData() const {
    if (IrReceiver.decode()) {
        consumeIrMessage(IrReceiver.decodedIRData.decodedRawData);
        // 准备接收下一个信号
        IrReceiver.resume();
    }
}
