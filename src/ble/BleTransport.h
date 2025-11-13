#pragma once
#include "../CommandContext.h"
#include "../CommandManager.h"

#include <BLEServer.h>
#include <map>

#include "freertos/task.h"

/************************************* BLEMessage *************************************/

struct BLEMessage {
    String id = "";
    String value = "";
};

class BleTransport;

/************************************* MyServerCallbacks *************************************/

class MyServerCallbacks : public BLEServerCallbacks {
private:
    BleTransport *owner = nullptr;

public:
    explicit MyServerCallbacks(BleTransport *owner);

    void onConnect(BLEServer *p) override;

    void onDisconnect(BLEServer *p) override;
};

/************************************* MyCallbacks *************************************/

class MyCallbacks : public BLECharacteristicCallbacks {
private:
    BleTransport *owner = nullptr;

public:
    explicit MyCallbacks(BleTransport *owner);

    void onWrite(BLECharacteristic *pCharacteristic) override;
};

/************************************* BleTransport *************************************/

class BleTransport : public CommandManager<CommandId_Type, CommandData_Type> {
private:
    BLEServer *p_Server = nullptr;
    BLECharacteristic *p_TxCharacteristic = nullptr;

    TaskHandle_t keepAliveTaskHandle = nullptr;
    TaskHandle_t reconnectTaskHandle = nullptr;

    String bleName;
    std::map<String, CommandId_Type> commandMapping;

    volatile bool isConnected = false;

    static void keepAliveTaskEntry(void *pvParameters);

    static void reconnectTaskEntry(void *pvParameters);

    void keepAliveTaskLoop();

    void reconnectTaskLoop();

public:
    explicit BleTransport(std::map<String, CommandId_Type> commandMapping,
                          String bleName = "NB-DIY");

    void setup();

    void consumeBleMessage(const BLEMessage &msg);

    void onBleConnect();

    void onBleDisconnect();
};
