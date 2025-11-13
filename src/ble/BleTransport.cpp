#include "BleTransport.h"

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLE2902.h>

#include <map>

#define SERVICE_UUID           "6E400011-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400012-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400013-B5A3-F393-E0A9-E50E24DCCA9E"

#define KEEP_CONNECT_INTERVAL_MS 3000
#define RECONNECT_INTERVAL_MS    1000

const String ALPHANUMERIC_CHARS = "abcdefghijklmnopqrstuvwxyz,ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";


/************************************* MyServerCallbacks *************************************/

MyServerCallbacks::MyServerCallbacks(BleTransport *owner)
    : owner(owner) {
}

void MyServerCallbacks::onConnect(BLEServer *p) {
    if (owner) {
        owner->onBleConnect();
    }
#ifdef LOG_DEBUG
    Serial.println("BleTransport onConnect");
#endif
}

void MyServerCallbacks::onDisconnect(BLEServer *p) {
    if (owner) {
        owner->onBleDisconnect();
    }
#ifdef LOG_DEBUG
    Serial.println("BleTransport onDisconnect");
#endif
}


/************************************* MyCallbacks *************************************/

MyCallbacks::MyCallbacks(BleTransport *owner)
    : owner(owner) {
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
            if (owner) {
                owner->consumeBleMessage(msg);
            }
        }
    }
}

/************************************* BleTransport *************************************/

BleTransport::BleTransport(std::map<String, CommandId_Type> commandMapping,
                           String bleName)
    : bleName(std::move(bleName)),
      commandMapping(std::move(commandMapping)) {
}

void BleTransport::setup() {
    // 把全局注册的 handler 拷贝到 CommandManager 的静态表里
    registerCommandHandlers(getGlobalCommandHandlers());

    BLEDevice::init(bleName.c_str());
    p_Server = BLEDevice::createServer();
    p_Server->setCallbacks(new MyServerCallbacks(this));

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

    pRxCharacteristic->setCallbacks(new MyCallbacks(this));

    pService->start();
    p_Server->getAdvertising()->start();

#ifdef LOG_DEBUG
    Serial.print("BleTransport::setup");
    Serial.print(", bleName: ");
    Serial.print(bleName);
    Serial.println();
#endif

    // 创建 FreeRTOS 任务：保持连接的心跳任务
    xTaskCreatePinnedToCore(
        BleTransport::keepAliveTaskEntry,
        "ble_keepalive",
        2048,
        this,
        5,
        &keepAliveTaskHandle,
        APP_CPU_NUM // 绑到 APP CPU（一般是 core 1）
    );

    // 创建 FreeRTOS 任务：断开后延时重启广播
    xTaskCreatePinnedToCore(
        BleTransport::reconnectTaskEntry,
        "ble_reconnect",
        2048,
        this,
        5,
        &reconnectTaskHandle,
        APP_CPU_NUM
    );
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


/************************************* FreeRTOS Task 实现 *************************************/

void BleTransport::keepAliveTaskEntry(void *pvParameters) {
    auto *self = static_cast<BleTransport *>(pvParameters);
    self->keepAliveTaskLoop();
}

void BleTransport::reconnectTaskEntry(void *pvParameters) {
    auto *self = static_cast<BleTransport *>(pvParameters);
    self->reconnectTaskLoop();
}

// 周期发送心跳的任务
void BleTransport::keepAliveTaskLoop() {
    uint8_t sendValue = 0;
    for (;;) {
        if (isConnected && p_TxCharacteristic != nullptr) {
            p_TxCharacteristic->setValue(&sendValue, 1);
            p_TxCharacteristic->notify();
            sendValue = (sendValue + 1) % 256;
        }
        vTaskDelay(pdMS_TO_TICKS(KEEP_CONNECT_INTERVAL_MS));
    }
}

// 断开连接后延时重新 startAdvertising 的任务
void BleTransport::reconnectTaskLoop() {
    for (;;) {
        // 等待通知：断开时会 xTaskNotifyGive()
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // 延时一段时间再重启广播
        vTaskDelay(pdMS_TO_TICKS(RECONNECT_INTERVAL_MS));

        if (p_Server != nullptr && !isConnected) {
            p_Server->startAdvertising();
#ifdef LOG_DEBUG
            Serial.println("BleTransport::reconnectTaskLoop startAdvertising");
#endif
        }
    }
}


/************************************* BLE 连接状态回调 *************************************/

void BleTransport::onBleConnect() {
    isConnected = true;
}

void BleTransport::onBleDisconnect() {
    isConnected = false;
    if (reconnectTaskHandle != nullptr) {
        xTaskNotifyGive(reconnectTaskHandle);
    }
}
