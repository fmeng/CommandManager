#define CUSTOM_SETTINGS
#define INCLUDE_LEDCONTROL_MODULE
#include <Arduino.h>
#include <DabbleESP32.h>
unsigned long lasttime = 0;

void setup() {
    Serial.begin(115200); // make sure your Serial Monitor is also set at this baud rate.
    Dabble.begin("MyEsp32"); //set bluetooth name of your device
}

void loop() {
    //this function is used to refresh data obtained from smartphone.Hence calling this function is mandatory in order to get data properly from your mobile.
    Dabble.processInput();

    static int lastPrintTime = millis();
    if (millis() - lastPrintTime > 1000) {
        Serial.print("Led:");
        Serial.print(LedControl.getpinNumber());
        Serial.print('\t');
        Serial.print("State:"); //0 if led is Off. 1 if led is On.
        Serial.print(LedControl.getpinState());
        Serial.print('\t');
        Serial.print("Brightness:");
        Serial.println(LedControl.readBrightness());
        lastPrintTime = millis();
    }
}
