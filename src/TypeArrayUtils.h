#pragma once

#include <Arduino.h>
#include <type_traits>
#include <string.h>
#include <string>
#include <utility>

template<typename T>
struct is_frame_data_value
        : std::integral_constant<bool,
            std::is_trivially_copyable<T>::value
            || std::is_same<String, T>::value
            || std::is_same<std::string, T>::value> {
};

template<typename T>
class TypeArrayUtils {
    static_assert(is_frame_data_value<T>::value,
                  "TypeArrayUtils<T>: T must be trivially copyable, Arduino String, or std::string");

public:
    static void obj2BytesCopy(const T &srcData, uint8_t *const p_dstData, const size_t dstDataSize = sizeof(T));

    static std::pair<const uint8_t * const, size_t> obj2BytesPoint(const T &srcData);

    static T bytes2ObjCopy(const uint8_t *const p_srcData, const size_t srcDataSize = sizeof(T));

    static const T *bytes2ObjPoint(const uint8_t *const p_srcData, const size_t srcDataSize = sizeof(T));
};


template<typename T>
void TypeArrayUtils<T>::obj2BytesCopy(const T &srcData, uint8_t *const p_dstData, const size_t dstDataSize) {
    static_assert(std::is_trivially_copyable<T>::value, "TypeUtils<T>: T must be trivially copyable");

    const size_t srcObjSize = sizeof(T);
    if (srcObjSize <= dstDataSize) {
        memcpy(p_dstData, &srcData, srcObjSize);
    } else {
#ifdef LOG_DEBUG
        Serial.print("TypeUtils<T>::obj2BytesCopy, no enough space");
        Serial.print(", srcObjSize: ");
        Serial.print(srcObjSize);
        Serial.print(", dstDataSize: ");
        Serial.print(dstDataSize);
        Serial.println();
#endif
    }
}

template<typename T>
std::pair<const uint8_t * const, size_t> TypeArrayUtils<T>::obj2BytesPoint(const T &srcData) {
    static_assert(std::is_trivially_copyable<T>::value, "TypeUtils<T>: T must be trivially copyable");
    return std::make_pair(reinterpret_cast<const uint8_t *>(&srcData), sizeof(T));
}

template<typename T>
T TypeArrayUtils<T>::bytes2ObjCopy(const uint8_t *const p_srcData, const size_t srcDataSize) {
    static_assert(std::is_trivially_copyable<T>::value, "TypeUtils<T>: T must be trivially copyable");
    const size_t dstObjSize = sizeof(T);
    T res{};
    if (dstObjSize <= srcDataSize) {
        memcpy(&res, p_srcData, dstObjSize);
    } else {
#ifdef LOG_DEBUG
        Serial.print("TypeUtils<T>::bytes2ObjCopy, no enough space");
        Serial.print(", srcDataSize: ");
        Serial.print(srcDataSize);
        Serial.print(", dstObjSize: ");
        Serial.print(dstObjSize);
        Serial.println();
#endif
    }
    return res;
}

template<typename T>
const T *TypeArrayUtils<T>::bytes2ObjPoint(const uint8_t *const p_srcData, const size_t srcDataSize) {
    static_assert(std::is_trivially_copyable<T>::value, "TypeUtils<T>: T must be trivially copyable");
    const size_t dstObjSize = sizeof(T);
    if (dstObjSize <= srcDataSize) {
        return reinterpret_cast<const T *>(p_srcData);
    } else {
#ifdef LOG_DEBUG
        Serial.print("TypeUtils<T>::bytes2ObjCopy, no enough space");
        Serial.print(", srcDataSize: ");
        Serial.print(srcDataSize);
        Serial.print(", dstObjSize: ");
        Serial.print(dstObjSize);
        Serial.println();
#endif
        return nullptr;
    }
}


// -------- TypeUtils<String> 特化 --------
template<>
class TypeArrayUtils<String> {
public:
    static void obj2BytesCopy(const String &srcData,
                              uint8_t *const p_dstData,
                              const size_t dstDataSize/*包含'\0',srcData.length()+1*/) {
        const size_t strLen = srcData.length();
        const size_t needSize = strLen + 1;
        if (needSize <= dstDataSize) {
            memcpy(p_dstData, srcData.c_str(), needSize);
        } else {
#ifdef LOG_DEBUG
            Serial.print("TypeUtils<String>::obj2BytesCopy, not enough space");
            Serial.print(", needSize: ");
            Serial.print(needSize);
            Serial.print(", dstDataSize: ");
            Serial.print(dstDataSize);
            Serial.println();
#endif
        }
    }

    static std::pair<const uint8_t * const, size_t/*包含'\0',srcData.length()+1*/> obj2BytesPoint(const String &srcData) {
        const size_t strLen = srcData.length();
        const size_t needSize = strLen + 1;
        return std::make_pair(reinterpret_cast<const uint8_t *>(srcData.c_str()), needSize);
    }

    static String bytes2ObjCopy(const uint8_t *const p_srcData,
                                const size_t srcDataSize/*包含'\0*/) {
        if (p_srcData == nullptr || srcDataSize == 0) {
            return String();
        }
        return String(reinterpret_cast<const char *>(p_srcData));
    }

    static const String *bytes2ObjPoint(const uint8_t *const, const size_t) = delete;
};


// -------- TypeUtils<std::string> --------
template<>
class TypeArrayUtils<std::string> {
public:
    static void obj2BytesCopy(const std::string &srcData, uint8_t *const p_dstData,
                              const size_t dstDataSize/*包含'\0',srcData.length()+1*/) {
        const size_t strLen = srcData.size();
        const size_t needSize = strLen + 1;
        if (needSize <= dstDataSize) {
            memcpy(p_dstData, srcData.data(), strLen);
            p_dstData[strLen] = '\0';
        } else {
#ifdef LOG_DEBUG
            Serial.print("TypeUtils<std::string>::obj2BytesCopy, not enough space");
            Serial.print(", needSize: ");
            Serial.print(needSize);
            Serial.print(", dstDataSize: ");
            Serial.print(dstDataSize);
            Serial.println();
#endif
        }
    }

    static std::pair<const uint8_t * const, size_t> obj2BytesPoint(const std::string &srcData) {
        const size_t strLen = srcData.size();
        const size_t needSize = strLen + 1; // 包含 '\0'
        return std::make_pair(reinterpret_cast<const uint8_t *>(srcData.c_str()), needSize);
    }

    static std::string bytes2ObjCopy(const uint8_t *const p_srcData,
                                     const size_t srcDataSize) {
        if (p_srcData == nullptr || srcDataSize == 0) {
            return std::string();
        }
        return std::string(reinterpret_cast<const char *>(p_srcData));
    }

    static const std::string *bytes2ObjPoint(const uint8_t *const, const size_t) = delete;
};
