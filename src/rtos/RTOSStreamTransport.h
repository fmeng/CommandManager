#pragma once
#include <Arduino.h>
#include <vector>
#include <freertos/message_buffer.h>
#include "FrameWrapper.h"
#include "rtos/RTOSTaskWrapper.h"

#ifndef DEFAULT_RX_BUFFER_ONE_MSG_MAX_SIZE
#define DEFAULT_RX_BUFFER_ONE_MSG_MAX_SIZE 128
#endif

#ifndef DEFAULT_TX_BUFFER_ONE_MSG_MAX_SIZE
#define DEFAULT_TX_BUFFER_ONE_MSG_MAX_SIZE 128
#endif

#ifndef DEFAULT_TX_BUFFER_ALL_MSG_MAX_SIZE
#define DEFAULT_TX_BUFFER_ALL_MSG_MAX_SIZE 1024
#endif

template<typename F>
class RTOSStreamTransport : public RTOSTaskWrapper {
    static_assert(std::is_base_of<FrameWrapper<typename F::value_type, F>, F>::value,
                  "UartTransport<F>: F must be FrameWrapper<T, F> or its CRTP-derived");
    static_assert(is_frame_data_value<typename F::value_type>::value,
                  "UartTransport<F>: value_type T is not supported by TypeUtils");

public:
    explicit RTOSStreamTransport(String taskName, Task_T setUpFun,
                             uint16_t const rxBufferOneMsgMaxSize = DEFAULT_RX_BUFFER_ONE_MSG_MAX_SIZE,
                             const uint16_t txBufferOneMsgMaxSize = DEFAULT_TX_BUFFER_ONE_MSG_MAX_SIZE,
                             const uint16_t txBufferAllMsgMaxSize = DEFAULT_TX_BUFFER_ALL_MSG_MAX_SIZE)
        : RTOSTaskWrapper(std::move(taskName), std::move(setUpFun), [this]() { this->loop(); }),
          m_rxBufferOneMsgMaxSize(rxBufferOneMsgMaxSize), m_txBufferOneMsgMaxSize(txBufferOneMsgMaxSize),
          m_txBufferAllMsgMaxSize(txBufferAllMsgMaxSize), m_rxBuff(), m_txBuff(txBufferOneMsgMaxSize) {
        // 只预留容量
        m_rxBuff.reserve(m_rxBufferOneMsgMaxSize);
        // 解决 lazyMessageBuffer 并发创建问题
        // m_sendMessageBuffer = xMessageBufferCreate(m_txBufferAllMsgMaxSize);
    }

    virtual void sendData(const uint8_t *const sendBuffer, const size_t sendSize) {
        if (sendBuffer == nullptr || sendSize <= 0) {
            return;
        }
        lazyMessageBuffer();
        const size_t xBytesSent = xMessageBufferSend(
            m_sendMessageBuffer,
            sendBuffer,
            sendSize,
            0
        );
#ifdef LOG_DEBUG
        if (xBytesSent == 0) {
            Serial.print(F("UartTransport::sendData: xBytesSent == 0"));
            Serial.println();
        }
#endif
    }

protected:
    const uint16_t m_rxBufferOneMsgMaxSize; // 接收串口数据, 每条消息的最大限制
    const uint16_t m_txBufferOneMsgMaxSize; // 发送串口数据, 每条消息的最大限制
    const uint16_t m_txBufferAllMsgMaxSize; // 发送串口数据, 所有消息的最大限制
    std::vector<uint8_t> m_rxBuff;
    std::vector<uint8_t> m_txBuff;
    Stream *p_stream = nullptr;
    MessageBufferHandle_t m_sendMessageBuffer = nullptr;

    virtual void lazyMessageBuffer() {
        if (m_sendMessageBuffer == nullptr) {
            m_sendMessageBuffer = xMessageBufferCreate(m_txBufferAllMsgMaxSize);
#ifdef LOG_DEBUG
            Serial.print(F("UartTransport::lazyMessageBuffer"));
            Serial.print(F(", m_txBufferAllMsgMaxSize: "));
            Serial.print(m_txBufferAllMsgMaxSize);
            Serial.println();
#endif
        }
    }

    virtual void consumeTxData() {
        if (p_stream == nullptr || m_sendMessageBuffer == nullptr) {
            return;
        }
        static F frameWrapper;
        uint8_t *const recvBuf = m_txBuff.data();
        size_t len = xMessageBufferReceive(m_sendMessageBuffer, recvBuf, m_txBufferOneMsgMaxSize, 0);
        while (len > 0) {
            const std::pair<const uint8_t * const, size_t> dataPire = frameWrapper.toTxSendData(recvBuf, len);
            p_stream->write(dataPire.first, dataPire.second);
#ifdef LOG_DEBUG
            Serial.print(F("UartTransport::consumeTxData"));
            Serial.print(F(", txDataSize: "));
            Serial.print(dataPire.second);
            Serial.println();
#endif
            len = xMessageBufferReceive(m_sendMessageBuffer, recvBuf, m_txBufferOneMsgMaxSize, 0);
        }
    }

    virtual void loop() {
        consumeTxData();
    }
};
