#pragma once
#include <map>
#include <functional>
#include "CommandIdEnum.h"


class CommandManager {
public:
    typedef std::function<void(CommandIdEnum commandId, uint8_t *p_commandValue, size_t valueSize)> CommandHandler;
    typedef std::function<void(uint32_t commandBits)> EventHandler;

    static CommandManager &instance() {
        static CommandManager instance_;
        return instance_;
    }

    CommandManager(const CommandManager &) = delete;

    CommandManager &operator=(const CommandManager &) = delete;

    CommandManager(CommandManager &&) = delete;

    CommandManager &operator=(CommandManager &&) = delete;

    void registerCommandHandler(const CommandIdEnum commandId, const CommandHandler &handler) {
        m_commandHandlers[commandId] = handler;
    }

    void registerEventHandler(const uint32_t cmdBits, const EventHandler &handler) {
        m_eventHandlers[cmdBits] = handler;
    }

    void handleCommand(const CommandIdEnum commandId, uint8_t *p_commandValue, const size_t valueSize) {
        const auto it = m_commandHandlers.find(commandId);
        if (it == m_commandHandlers.end()) {
            return;
        }
        it->second(commandId, p_commandValue, valueSize);
    }

    std::map<uint32_t, EventHandler> &getEventHandlers() {
        return m_eventHandlers;
    }

private:
    CommandManager() {
    }

    std::map<CommandIdEnum, CommandHandler> m_commandHandlers;
    std::map<uint32_t, EventHandler> m_eventHandlers;
};

struct RegisterCommandHandler {
    RegisterCommandHandler(const CommandIdEnum commandId, const CommandManager::CommandHandler &handler) {
        CommandManager::instance().registerCommandHandler(commandId, handler);
    }
};

struct RegisterEventHandler {
    RegisterEventHandler(const uint32_t cmdBits, const CommandManager::EventHandler &handler) {
        CommandManager::instance().registerEventHandler(cmdBits, handler);
    }
};

#define CONCAT_INNER(a, b) a##b
#define CONCAT(a, b) CONCAT_INNER(a, b)
#define UNIQUE_NAME(base) CONCAT(base, __COUNTER__)

#define REGISTER_COMMAND_HANDLER(id, fn)                              \
namespace {                                                           \
static RegisterCommandHandler UNIQUE_NAME(_auto_cmd_reg_) ( id, fn ); \
}

#define REGISTER_EVENT_HANDLER(id, fn)                              \
namespace {                                                         \
static RegisterEventHandler UNIQUE_NAME(_auto_evt_reg_) ( id, fn ); \
}
