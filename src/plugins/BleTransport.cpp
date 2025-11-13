#include "BleTransport.h"
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLE2902.h>
#include <functional>
#include <map>

#define SERVICE_UUID          "6E400011-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400012-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400013-B5A3-F393-E0A9-E50E24DCCA9E"

#define KEEP_CONNECT_INTERVAL_MS 3000
#define RECONNECT_INTERVAL_MS 1000

const String ALPHANUMERIC_CHARS = "abcdefghijklmnopqrstuvwxyz,ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";


/************************************* MyServerCallbacks *************************************/

MyServerCallbacks::MyServerCallbacks(Task *p_keepConnectDataTask,
                                     Task *p_reconnectTask)
    : p_keepConnectDataTask(p_keepConnectDataTask),
      p_reconnectTask(p_reconnectTask) {
}

void MyServerCallbacks::onConnect(BLEServer *p) {
    p_keepConnectDataTask->enableIfNot();
#ifdef LOG_DEBUG
    Serial.println("BleTransport onConnect");
#endif
}

void MyServerCallbacks::onDisconnect(BLEServer *p) {
    p_keepConnectDataTask->disable();
    p_reconnectTask->restartDelayed(RECONNECT_INTERVAL_MS);
#ifdef LOG_DEBUG
    Serial.println("BleTransport onDisconnect");
#endif
}

/************************************* MyCallbacks *************************************/

MyCallbacks::MyCallbacks(std::function<void(BLEMessage &)> consume)
    : consume(std::move(consume)) {
}

void MyCallbacks::onWrite(BLECharacteristic *pCharacteristic) {
    String message = pCharacteristic->getValue().c_str();
    BLEMessage msg;
    int counter = 0;

    for (int i = 0; i < message.length(); i++) {
        if (ALPHANUMERIC_CHARS.indexOf(message[i]) == -1) {
            counter++;
        }
        if (counter == 1 && i > 0) {
            msg.id.concat(message[i]);
        } else if (counter == 2 && i > msg.id.length() + 1) {
            msg.value.concat(message[i]);
        } else if (msg.id.length() > 0 && msg.value.length() > 0) {
            consume(msg);
        }
    }
}

/************************************* BleTransport *************************************/

BleTransport::BleTransport(std::map<String, CommandId_Type> commandMapping,
                           Scheduler &runner,
                           String bleName)
    : commandMapping(std::move(commandMapping)),
      runner(runner),
      bleName(std::move(bleName)) {
}

void BleTransport::setup() {
    registerCommandHandlers(getGlobalCommandHandlers());

    p_keepConnectDataTask = new Task(
        KEEP_CONNECT_INTERVAL_MS,
        TASK_FOREVER,
        [this]() {
            static uint8_t sendValue = 0;
            if (p_TxCharacteristic == nullptr) return;

            p_TxCharacteristic->setValue(&sendValue, 1);
            p_TxCharacteristic->notify();
            sendValue = (sendValue + 1) % 256;
        },
        &runner,
        false
    );

    p_reconnectTask = new Task(
        0, TASK_ONCE,
        [this]() {
            if (p_Server == nullptr) return;
            p_Server->startAdvertising();
        },
        &runner,
        false
    );

    BLEDevice::init(bleName.c_str());
    p_Server = BLEDevice::createServer();
    p_Server->setCallbacks(new MyServerCallbacks(p_keepConnectDataTask, p_reconnectTask));

    BLEService *pService = p_Server->createService(SERVICE_UUID);

    p_TxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY
    );

    p_TxCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE
    );

    pRxCharacteristic->setCallbacks(
        new MyCallbacks([this](BLEMessage &m) { this->consumeBleMessage(m); })
    );

    pService->start();
    p_Server->getAdvertising()->start();

#ifdef LOG_DEBUG
    Serial.print("BleTransport::setup");
    Serial.print(", bleName: ");
    Serial.print(bleName);
    Serial.println();
#endif
}

void BleTransport::consumeBleMessage(const BLEMessage &msg) {
    const String key = msg.id;
    if (key == nullptr || key == "") {
        return;
    }
    String value = msg.value;
    if (value == nullptr || value == "") {
        return;
    }
    if (commandMapping.find(key) == commandMapping.end()) {
#ifdef LOG_DEBUG
        Serial.print("BleTransport::consumeBleMessage");
        Serial.print(", no mapping, ");
        Serial.print(", key: ");
        Serial.print(key);
        Serial.print(", value: ");
        Serial.print(value);
        Serial.println();
#endif
        return;
    }
    const CommandId_Type commandId = commandMapping[key];
    const bool res = handleCommand(commandId, *p_commandData, &value);
#ifdef LOG_DEBUG
    Serial.print("BleTransport::consumeBleMessage");
    Serial.print(", res: ");
    Serial.print(res);
    Serial.print(", key: ");
    Serial.print(key);
    Serial.print(", value: ");
    Serial.print(value);
    Serial.print(", commandId: ");
    Serial.print(static_cast<int>(commandId));
    Serial.println();
#endif
}
