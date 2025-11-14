#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <Arduino.h>
#include <DabbleESP32.h>
#include <map>
#include <functional>

// --------------------------------------------------
// Keys in fixed display order
// --------------------------------------------------
std::vector<String> keysInOrder = {
    "Up", "Down", "Left", "Right", "Square", "Circle", "Cross",
    "Triangle", "Start", "Select", "Angle", "Radius", "Xaxis", "Yaxis"
};

// --------------------------------------------------
// Function map
// --------------------------------------------------
std::map<String, std::function<String(GamePadModule &)> > callMap;

// --------------------------------------------------
std::map<String, String> lastValueMap;
std::map<String, String> nowValueMap;

// --------------------------------------------------
// Safe print (handles missing keys & no crash)
// --------------------------------------------------
void printMap(const std::map<String, String> &m) {
    if (m.empty()) {
        Serial.println("empty map");
        return;
    }

    bool first = true;
    for (const auto &k: keysInOrder) {
        if (!first)
            Serial.print(",");
        first = false;

        if (!first)
            Serial.print(" ");
        Serial.print(k);
        Serial.print(":");

        auto it = m.find(k);
        if (it == m.end())
            Serial.print("NA");
        else
            Serial.print(it->second);
    }
    Serial.println();
}

// --------------------------------------------------
void setup() {
    Serial.begin(115200);
    Dabble.begin("MyEsp32");

    // Boolean keys
    callMap["Up"] = [](GamePadModule &p) { return p.isUpPressed() ? "1" : "0"; };
    callMap["Down"] = [](GamePadModule &p) { return p.isDownPressed() ? "1" : "0"; };
    callMap["Left"] = [](GamePadModule &p) { return p.isLeftPressed() ? "1" : "0"; };
    callMap["Right"] = [](GamePadModule &p) { return p.isRightPressed() ? "1" : "0"; };
    callMap["Square"] = [](GamePadModule &p) { return p.isSquarePressed() ? "1" : "0"; };
    callMap["Circle"] = [](GamePadModule &p) { return p.isCirclePressed() ? "1" : "0"; };
    callMap["Cross"] = [](GamePadModule &p) { return p.isCrossPressed() ? "1" : "0"; };
    callMap["Triangle"] = [](GamePadModule &p) { return p.isTrianglePressed() ? "1" : "0"; };
    callMap["Start"] = [](GamePadModule &p) { return p.isStartPressed() ? "1" : "0"; };
    callMap["Select"] = [](GamePadModule &p) { return p.isSelectPressed() ? "1" : "0"; };

    // Numeric
    callMap["Angle"] = [](GamePadModule &p) { return String(p.getAngle()); };
    callMap["Radius"] = [](GamePadModule &p) { return String(p.getRadius()); };
    callMap["Xaxis"] = [](GamePadModule &p) { return String(p.getXaxisData(), 2); };
    callMap["Yaxis"] = [](GamePadModule &p) { return String(p.getYaxisData(), 2); };
}

// --------------------------------------------------
void fillMap(std::map<String, String> &m) {
    for (const auto &k: keysInOrder) {
        auto it = callMap.find(k);
        if (it != callMap.end()) {
            m[k] = it->second(GamePad);
        }
    }
}

// --------------------------------------------------
bool anyValueChanged(const std::map<String, String> &m0,
                     const std::map<String, String> &m1) {
    for (const auto &k: keysInOrder) {
        auto it0 = m0.find(k);
        auto it1 = m1.find(k);

        if (it0 == m0.end() || it1 == m1.end())
            return true;

        if (it0->second != it1->second)
            return true;
    }
    return false;
}

// --------------------------------------------------
void loop() {
    Dabble.processInput();

    fillMap(nowValueMap);

    if (lastValueMap.empty()) {
        lastValueMap = nowValueMap;
        printMap(lastValueMap);
        return;
    }

    if (anyValueChanged(lastValueMap, nowValueMap)) {
        lastValueMap = nowValueMap;
        printMap(lastValueMap);
    }
}
