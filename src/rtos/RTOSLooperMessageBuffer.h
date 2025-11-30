#pragma once

#include <freertos/message_buffer.h>
#include "CommandManager.h"

#ifndef LOOPER_MESSAGE_BUFFER_SIZE
#define LOOPER_MESSAGE_BUFFER_SIZE 1024
#endif

#ifndef LOOPER_ONE_MESSAGE_SIZE
#define LOOPER_ONE_MESSAGE_SIZE 256
#endif

class RTOSLooperMessageBuffer {
public:
    static RTOSLooperMessageBuffer &instance() {
        static RTOSLooperMessageBuffer instance_;
        return instance_;
    }

    RTOSLooperMessageBuffer(const RTOSLooperMessageBuffer &) = delete;

    RTOSLooperMessageBuffer &operator=(const RTOSLooperMessageBuffer &) = delete;

    RTOSLooperMessageBuffer(RTOSLooperMessageBuffer &&) = delete;

    RTOSLooperMessageBuffer &operator=(RTOSLooperMessageBuffer &&) = delete;

    MessageBufferHandle_t &getMessageBuffer() {
        lazyCreateMessageBuffer();
        return m_messageBuffer;
    }

    void loop() {
        lazyCreateMessageBuffer();

        static uint8_t recvBuf[LOOPER_ONE_MESSAGE_SIZE];

        const size_t len = xMessageBufferReceive(m_messageBuffer, recvBuf, sizeof(recvBuf), 0);
        if (len <= 0) {
            return;
        }
        const auto commandId = static_cast<CommandIdEnum>(recvBuf[0]);
        if (len > 1) {
            CommandManager::instance().handleCommand(commandId, recvBuf + 1, len - 1);
        } else {
            CommandManager::instance().handleCommand(commandId, nullptr, 0);
        }
    }

private:
    MessageBufferHandle_t m_messageBuffer;

    RTOSLooperMessageBuffer() : m_messageBuffer(nullptr) {
    }

    void lazyCreateMessageBuffer() {
        if (m_messageBuffer == nullptr) {
            m_messageBuffer = xMessageBufferCreate(LOOPER_MESSAGE_BUFFER_SIZE);
        }
    }
};
