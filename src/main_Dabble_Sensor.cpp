#define CUSTOM_SETTINGS
#define INCLUDE_SENSOR_MODULE
#include <Arduino.h>
#include <DabbleESP32.h>
#include <map>
#include <functional>

// --------------------------------------------------
// Keys in fixed display order
// --------------------------------------------------
std::vector<String> keysInOrder = {
    "AccX", "AccY", "AccZ", "GyrX", "GyrY", "GyrZ", "MagX",
    "MagY", "MagZ", "Sound", "long", "lati", "Pressure"
};

// --------------------------------------------------
// Function map
// --------------------------------------------------
std::map<String, std::function<String(SensorModule &)> > callMap;

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


// Sensor.getLightIntensity()
// Sensor.getProximityDistance()
// Sensor.getTemperature()

    // 加速度
    callMap["AccX"] = [](SensorModule &s) { return String(s.getAccelerometerXaxis(), 4); };
    callMap["AccY"] = [](SensorModule &s) { return String(s.getAccelerometerYaxis(), 4); };
    callMap["AccZ"] = [](SensorModule &s) { return String(s.getAccelerometerZaxis(), 4); };

    // 陀螺仪
    callMap["GyrX"] = [](SensorModule &s) { return String(s.getGyroscopeXaxis(), 3); };
    callMap["GyrY"] = [](SensorModule &s) { return String(s.getGyroscopeYaxis(), 3); };
    callMap["GyrZ"] = [](SensorModule &s) { return String(s.getGyroscopeZaxis(), 3); };

    // 磁力计
    callMap["MagX"] = [](SensorModule &s) { return String(s.getMagnetometerXaxis(), 3); };
    callMap["MagY"] = [](SensorModule &s) { return String(s.getMagnetometerYaxis(), 3); };
    callMap["MagZ"] = [](SensorModule &s) { return String(s.getMagnetometerZaxis(), 3); };

    // 声音
    callMap["Sound"] = [](SensorModule &s) { return String(s.getSoundDecibels(), 3); };

    // 经纬度
    callMap["long"] = [](SensorModule &s) { return String(s.getGPSlongitude(), 2); };
    callMap["lati"] = [](SensorModule &s) { return String(s.getGPSLatitude(), 2); };

    // 压强
    callMap["Pressure"] = [](SensorModule &s) { return String(s.getBarometerPressure(), 2); };
}

// --------------------------------------------------
void fillMap(std::map<String, String> &m) {
    for (const auto &k: keysInOrder) {
        auto it = callMap.find(k);
        if (it != callMap.end()) {
            m[k] = it->second(Sensor);
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
