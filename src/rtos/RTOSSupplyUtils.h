#pragma once
#include <Arduino.h>
#include "rtos/RTOSLooperMessageBuffer.h"
#include "rtos/RTOSLooperEventGroup.h"
#include "CommandBitsUtil.h"
#include "CommandIdEnum.h"

class RTOSSupplyUtils final {
public:
    static void sendMessageBufferTask(const uint8_t *const p_commandValue, const size_t valueSize) {
        if (valueSize <= 0 || p_commandValue == nullptr) {
            return;
        }
        const size_t xBytesSent = xMessageBufferSend(
            RTOSLooperMessageBuffer::instance().getMessageBuffer(),
            p_commandValue,
            valueSize,
            0
        );
#ifdef LOG_DEBUG
        if (xBytesSent != valueSize) {
            Serial.print("RTOSSupplyUtils::sendMessageBufferTask, not enough space");
            Serial.print(", valueSize: ");
            Serial.print(valueSize);
            Serial.print(", xBytesSent: ");
            Serial.print(xBytesSent);
        }
#endif
    }

    static void sendMessageBufferTask(const CommandIdEnum commandId, const uint8_t *const p_commandValue,
                                      const size_t valueSize) {
        if (valueSize <= 0 || p_commandValue == nullptr) {
            return;
        }
        std::vector<uint8_t> frame;
        frame.reserve(valueSize + 1);
        frame.push_back(commandId);
        frame.insert(frame.end(), p_commandValue, p_commandValue + valueSize);
        const size_t totalSize = frame.size();
        sendMessageBufferTask(frame.data(), totalSize);
    }

    static void sendEventGroupTask(const CommandIdEnum commandId) {
        const EventBits_t setBits = commandsToEventBits(commandId);
        const EventBits_t oldBits = xEventGroupSetBits(RTOSLooperEventGroup::instance().getEventGroup(), setBits);
#ifdef LOG_DEBUG
        if ((oldBits & setBits) == 0) {
            Serial.print("RTOSSupplyUtils::sendEventGroupTask, first set bits");
            Serial.print(", setBits: ");
            Serial.print(setBits);
            Serial.print(", oldBits: ");
            Serial.print(oldBits);
            Serial.println();
        }
#endif
    }

    static void IRAM_ATTR sendEventGroupISR(const CommandIdEnum commandId) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        const BaseType_t xResult = xEventGroupSetBitsFromISR(RTOSLooperEventGroup::instance().getEventGroup(),
                                                             commandsToEventBits(commandId),
                                                             &xHigherPriorityTaskWoken);

        if (xResult == pdPASS) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
};
