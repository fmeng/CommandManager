// arkhipenko/TaskScheduler @ ^3.8.5

#include <Arduino.h>
#include "plugins/BleTransport.h"


Scheduler runner;

BleTransport bleTransport({
                              {"on", CommandIdEnum::COMMAND_ID_TURN_ON},
                              {"off", CommandIdEnum::COMMAND_ID_TURN_OFF},
                              {"left", CommandIdEnum::COMMAND_ID_TURN_LEFT},
                          }, runner, "NB-DIY");


void turnOnLed(CommandIdEnum commandId, CommandData &command_data, void *p_attach) {
    const String value = *static_cast<String *>(p_attach);
    Serial.print("执行回调");
    Serial.print(", commandId: ");
    Serial.print(static_cast<uint8_t>(commandId));
    Serial.print(", value: ");
    Serial.print(value);
    Serial.println();
}

REGISTER_HANDLER(CommandIdEnum::COMMAND_ID_TURN_ON, turnOnLed);

void setup() {
    Serial.begin(115200);
    delay(1000);
    bleTransport.setup();
    runner.enable();
}

void loop() {
    runner.execute();
}
