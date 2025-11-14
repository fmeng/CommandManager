#include <Arduino.h>
#include "ir/IRTransport.h"
#include <map>

#define KEY_MSG_PRE   0xBB44FF00
#define KEY_MSG_NEXT  0xBF40FF00
#define KEY_MSG_PAUSE 0xBC43FF00

IRTransport irRTransport(18, {
                             {KEY_MSG_PRE, CommandIdEnum::COMMAND_ID_TURN_ON},
                             {KEY_MSG_NEXT, CommandIdEnum::COMMAND_ID_TURN_OFF},
                             {KEY_MSG_PAUSE, CommandIdEnum::COMMAND_ID_TURN_LEFT},
                         });


void turnOnLed(CommandIdEnum commandId, CommandData &command_data, void *p_attach) {
    const String value = *static_cast<String *>(p_attach);
    Serial.print("执行回调");
    Serial.print(", commandId: ");
    Serial.print(static_cast<uint8_t>(commandId));
    Serial.print(", value: ");
    Serial.print(value);
    Serial.println();
    command_data.accessMutex([](CommandData &data) {
        data.id = millis() % 1000;
        Serial.print("安全更新 data ");
        Serial.print(", id: ");
        Serial.print(data.id);
        Serial.println();
    });
    CommandData &c = command_data.dump();
    Serial.print("获取副本 c ");
    Serial.print(", id: ");
    Serial.print(c.id);
    Serial.println();
}


void setup() {
    Serial.begin(115200);
    delay(1000);
    REGISTER_HANDLER(CommandIdEnum::COMMAND_ID_TURN_ON, turnOnLed);
    irRTransport.setup();
}

void loop() {
}
