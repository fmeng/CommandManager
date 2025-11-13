#pragma once
#include "../CommandContext.h"
#include "../CommandManager.h"
#include <BLEServer.h>

#define _TASK_INLINE
#ifndef _TASK_STD_FUNCTION
#define _TASK_STD_FUNCTION
#endif

#include <TaskScheduler.h>

struct BLEMessage {
    String id = "";
    String value = "";
};

class MyServerCallbacks : public BLEServerCallbacks {
private:
    Task *p_keepConnectDataTask;
    Task *p_reconnectTask;

public:
    MyServerCallbacks(Task *p_keepConnectDataTask,
                      Task *p_reconnectTask);

    void onConnect(BLEServer *p) override;

    void onDisconnect(BLEServer *p) override;
};

class MyCallbacks : public BLECharacteristicCallbacks {
    std::function<void(BLEMessage &)> consume;

public:
    explicit MyCallbacks(std::function<void(BLEMessage &)> consume);

    void onWrite(BLECharacteristic *pCharacteristic) override;
};

// CommandContext.h
extern std::map<CommandId_Type, CommandHandler> &getGlobalCommandHandlers();

class BleTransport : public CommandManager<CommandId_Type, CommandData_Type> {
private:
    BLEServer *p_Server = nullptr;
    BLECharacteristic *p_TxCharacteristic = nullptr;
    Task *p_keepConnectDataTask = nullptr;
    Task *p_reconnectTask = nullptr;

    Scheduler &runner;
    String bleName;
    std::map<String, CommandId_Type> commandMapping;

public:
    BleTransport(std::map<String, CommandId_Type> commandMapping,
                 Scheduler &runner,
                 String bleName = "NB-DIY");

    void setup();

    void consumeBleMessage(const BLEMessage &msg);
};
