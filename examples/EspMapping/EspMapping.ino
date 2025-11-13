/**
 * Demo For EspMapping
 *
 * @brief
 * @author fmeng fmeng123@gmail.com
 * @version 1.0
 * @date 2025/10/25
 * @copyright 奶爸DIY Copyright (c) 2025
 */
#include <Arduino.h>
#include <EspMapping.h>

void setup() {
    Serial.begin(115200);
}

void loop() {
    Serial.println(MP_MCU_NAME);
    delay(1000);
}