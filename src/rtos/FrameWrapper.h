#pragma once
#include <Arduino.h>
#include "CommandIdEnum.h"
#include "TypeArrayUtils.h"

/**
 * CRTP , no virtual, delegate
 */
template<typename T, typename Derived>
class FrameWrapper {
    static_assert(is_frame_data_value<T>::value,
                  "FrameWrapper<T, Derived> T must be trivially copyable, Arduino String, or std::string");

protected:
    CommandIdEnum commandId = static_cast<CommandIdEnum>(0);
    T frame;

public:
    using value_type = T; // type holder

    /**
     * 查找帧头
     *
     * @param p_buff
     * @param size
     * @return 数据帧头索引(第一个填充frame对象的有效索引数据、包含), 小于0未找到索引
     */
    int findFrameHeadIncludeIndex(const uint8_t *const p_buff, const size_t size) const {
        if (p_buff == nullptr || size == 0) {
            return -1;
        }
        return static_cast<const Derived &>(*this).findFrameHeadIncludeIndex(p_buff, size);
    }

    /**
     * 查找帧尾
     *
     * @param p_buff
     * @param size
     * @param skipIndex
     * @return 数据帧头索引(最后一个填充frame对象的有效索引数据、包含), 小于0未找到索引
     */
    int findFrameTailIncludeIndex(const uint8_t *const p_buff, const size_t size, const size_t skipIndex) const {
        if (p_buff == nullptr || size == 0) {
            return -1;
        }
        return static_cast<const Derived &>(*this).findFrameTailIncludeIndex(p_buff, size, skipIndex);
    }

    /**
     * 使用字节数组构建frame对象
     *
     * @param p_buff
     * @param size
     * @param headIncludeIndex
     * @param tailIncludeIndex
     * @return 返回填充对象消耗的字节数量，小于0未消耗任何字节
     */
    int fillFrameData(const uint8_t *const p_buff, const size_t size, const size_t headIncludeIndex,
                      const size_t tailIncludeIndex) {
        if (p_buff == nullptr || size == 0) {
            return -1;
        }
        return static_cast<Derived &>(*this).fillFrameData(p_buff, size, headIncludeIndex, tailIncludeIndex);
    }

    /**
     * 收到数据传送给串口,变换结构
     *
     * @param p_buff
     * @param size
     * @return <数组指针, 数组大小>
     */
    std::pair<const uint8_t * const, size_t> toTxSendData(const uint8_t *const p_buff, const size_t size) const {
        if (p_buff == nullptr || size == 0) {
            return std::make_pair(nullptr, 0);
        }
        return static_cast<const Derived &>(*this).toTxSendData(p_buff, size);
    }

    /**
     * frame数据变换结构传送给loop的message buffer
     *
     * @return @return <数组指针, 数组大小>
     */
    std::pair<const uint8_t * const, size_t> toLoopMessageBufferData() const {
        return static_cast<const Derived &>(*this).toLoopMessageBufferData();
    }

    const T &getFrame() const {
        return frame;
    }

    CommandIdEnum getCommandId() const {
        return commandId;
    }
};
