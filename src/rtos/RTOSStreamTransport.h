#pragma once
#include <Arduino.h>
#include <vector>
#include <freertos/message_buffer.h>
#include "FrameWrapper.h"
#include "RTOSSupplyUtils.h"
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
    // static_assert(is_frame_data_value<typename F::value_type>::value,
    //               "UartTransport<F>: value_type T is not supported by TypeUtils");

public:
    explicit RTOSStreamTransport(String taskName, Task_T setUpFun,
                                 uint16_t const rxBufferOneMsgMaxSize = DEFAULT_RX_BUFFER_ONE_MSG_MAX_SIZE,
                                 const uint16_t txBufferOneMsgMaxSize = DEFAULT_TX_BUFFER_ONE_MSG_MAX_SIZE,
                                 const uint16_t txBufferAllMsgMaxSize = DEFAULT_TX_BUFFER_ALL_MSG_MAX_SIZE)
        : RTOSTaskWrapper(std::move(taskName), std::move(setUpFun), [this]() { this->loop(); }),
          m_rxBufferOneMsgMaxSize(rxBufferOneMsgMaxSize), m_txBufferOneMsgMaxSize(txBufferOneMsgMaxSize),
          m_txBufferAllMsgMaxSize(txBufferAllMsgMaxSize), m_rxBuff(),
          m_txBuff(new uint8_t[DEFAULT_TX_BUFFER_ONE_MSG_MAX_SIZE]) {
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
    const uint16_t m_rxBufferOneMsgMaxSize; // 接收数据, 每条消息的最大限制
    const uint16_t m_txBufferOneMsgMaxSize; // 发送数据, 每条消息的最大限制
    const uint16_t m_txBufferAllMsgMaxSize; // 发送数据, 所有消息的最大限制
    std::vector<uint8_t> m_rxBuff;
    uint8_t *m_txBuff;
    MessageBufferHandle_t m_sendMessageBuffer = nullptr;

    // p_stream->available()
    virtual int available() = 0;

    // p_stream->readBytes(m_rxBuff.data() + oldSize, readySize);
    virtual size_t readBytes(uint8_t *buffer, size_t length) = 0;

    // p_stream->write(m_txBuff, len);
    virtual size_t write(const uint8_t *buffer, size_t size) = 0;

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

    virtual int receiveRxData() {
        const int readySize = available();
        if (readySize <= 0) {
            return -1;
        }

        const size_t oldSize = m_rxBuff.size();
        const size_t newSize = oldSize + readySize;

        if (newSize > m_rxBuff.capacity()) {
            m_rxBuff.reserve(newSize);
        }

        m_rxBuff.resize(newSize);

        const size_t actuallyRead = readBytes(m_rxBuff.data() + oldSize, readySize);
        if (actuallyRead < static_cast<size_t>(readySize)) {
            m_rxBuff.resize(oldSize + actuallyRead);
        }

        return actuallyRead;
    }

    virtual void consumeRxData() {
        static F frameWrapper;
        int headIncludeIndex = frameWrapper.findFrameHeadIncludeIndex(m_rxBuff.data(), m_rxBuff.size());
        while (headIncludeIndex >= 0) {
            const int tailIncludeIndex = frameWrapper.
                    findFrameTailIncludeIndex(m_rxBuff.data(), m_rxBuff.size(),
                                              headIncludeIndex);
            if (tailIncludeIndex < 0) {
                return;
            }

            // 填充 frame对象
            frameWrapper.fillFrameData(m_rxBuff.data(), m_rxBuff.size(), headIncludeIndex, tailIncludeIndex);

            // 构建序列化数组
            const std::pair<const uint8_t * const, size_t> dataPire = frameWrapper.toLoopMessageBufferData();

            // 发送数据
            RTOSSupplyUtils::sendMessageBufferTask(dataPire.first, dataPire.second);

#ifdef LOG_DEBUG
            Serial.print(F("UartTransport::consumeRxData"));
            Serial.print(F(", rxDataSize: "));
            Serial.print(dataPire.second);
            Serial.println();
#endif

            // 擦除m_rxBuff
            this->m_rxBuff.erase(this->m_rxBuff.begin(), this->m_rxBuff.begin() + tailIncludeIndex + 1);

            // 继续判断
            headIncludeIndex = frameWrapper.findFrameHeadIncludeIndex(m_rxBuff.data(),
                                                                      m_rxBuff.size());
        }
    }

    virtual void consumeTxData() {
        if (m_sendMessageBuffer == nullptr) {
            return;
        }
        size_t len = xMessageBufferReceive(m_sendMessageBuffer, m_txBuff, m_txBufferOneMsgMaxSize, 0);
        while (len > 0) {
            const size_t writeSize = write(m_txBuff, len);
#ifdef LOG_DEBUG
            Serial.print(F("UartTransport::consumeTxData"));
            if (writeSize != len) {
                Serial.print(F(", writeSize != len , writeSize: "));
                Serial.print(writeSize);
            }
            Serial.print(F(", txDataSize: "));
            Serial.print(len);
            Serial.println();
#endif
            len = xMessageBufferReceive(m_sendMessageBuffer, m_txBuff, m_txBufferOneMsgMaxSize, 0);
        }
    }

    virtual void loop() {
        if (receiveRxData() > 0) {
            consumeRxData();
        }
        consumeTxData();
    }
};
