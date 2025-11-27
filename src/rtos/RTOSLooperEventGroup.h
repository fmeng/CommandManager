#pragma once

#include "CommandManager.h"

class RTOSLooperEventGroup {
public:
    static RTOSLooperEventGroup &instance() {
        static RTOSLooperEventGroup instance_;
        return instance_;
    }

    RTOSLooperEventGroup(const RTOSLooperEventGroup &) = delete;

    RTOSLooperEventGroup &operator=(const RTOSLooperEventGroup &) = delete;

    RTOSLooperEventGroup(RTOSLooperEventGroup &&) = delete;

    RTOSLooperEventGroup &operator=(RTOSLooperEventGroup &&) = delete;

    EventGroupHandle_t &getEventGroup() {
        lazyCreateEventGroup();
        return m_eventGroup;
    }

    void loop() {
        lazyCreateEventGroup();
        uint32_t handleBits = 0;
        std::map<uint32_t, CommandManager::EventHandler> eventHandlers = CommandManager::instance().getEventHandlers();
        for (auto &it: eventHandlers) {
            const uint32_t commandBits = it.first;
            CommandManager::EventHandler &handler = it.second;
            const EventBits_t recvBits = xEventGroupWaitBits(
                m_eventGroup,
                commandBits,
                pdFALSE,
                pdTRUE,
                0 // 不阻塞
            );
            if ((recvBits & commandBits) == commandBits) {
                handler(commandBits);
                handleBits = handleBits | commandBits;
            }
        }

        if (handleBits) {
            xEventGroupClearBits(m_eventGroup, handleBits);
        }
    }

private:
    EventGroupHandle_t m_eventGroup;

    RTOSLooperEventGroup() : m_eventGroup(nullptr) {
    }

    void lazyCreateEventGroup() {
        if (m_eventGroup == nullptr) {
            m_eventGroup = xEventGroupCreate();
        }
    }
};
