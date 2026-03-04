/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _I2C_BUS_H_
#define _I2C_BUS_H_

// ============================================================
//  Platform Detection & Compatibility Layer
// ============================================================
#ifdef ARDUINO

#include <Arduino.h>
#include <Wire.h>

#else
// Native ESP-IDF environment

#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdarg>

#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* ================= Arduino Compatibility Macros ================= */

/**
 * @brief Provide Arduino-like delay() in ESP-IDF.
 */
#ifndef delay
#define delay(ms) vTaskDelay(pdMS_TO_TICKS(ms))
#endif

/**
 * @brief Provide Arduino-like millis() in ESP-IDF.
 */
#ifndef millis
#define millis() ((uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS))
#endif

/* ================= Serial Compatibility Layer ================= */

#ifndef SERIAL_COMPAT_DEFINED
#define SERIAL_COMPAT_DEFINED

/**
 * @brief Minimal Arduino-style Serial compatibility for ESP-IDF.
 *
 * Redirects output to ESP-IDF logging system.
 */
class SerialCompat {
public:
    void printf(const char* fmt, ...) __attribute__((format(printf, 2, 3)))
    {
        char buf[256];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        esp_log_write(ESP_LOG_INFO, "DRV", "%s", buf);
    }

    void println(const char* s = "")
    {
        ESP_LOGI("DRV", "%s", s);
    }

    void print(const char* s)
    {
        esp_log_write(ESP_LOG_INFO, "DRV", "%s", s);
    }
};

// Inline instance to avoid multiple definition
inline SerialCompat Serial;

#endif  // SERIAL_COMPAT_DEFINED
#endif  // ARDUINO

// ============================================================
//  I2C Debug Switch
// ============================================================
// #define I2C_DEBUG

#ifdef I2C_DEBUG

#ifdef ARDUINO
#define I2C_PRINT(...)   Serial.print(__VA_ARGS__)
#define I2C_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define I2C_PRINT(fmt, ...)   ESP_LOGI("I2C", fmt, ##__VA_ARGS__)
#define I2C_PRINTLN(fmt, ...) ESP_LOGI("I2C", fmt, ##__VA_ARGS__)
#endif

#else
#define I2C_PRINT(...)
#define I2C_PRINTLN(...)
#endif

// ============================================================
//  I2CBus Class
// ============================================================

#ifndef ARDUINO
#define I2C_DEV_CACHE_MAX 8
#endif

/**
 * @brief Cross-platform I2C abstraction layer.
 *
 * Supports:
 *  - Arduino (TwoWire)
 *  - ESP-IDF (i2c_master driver v5.x)
 *
 * Features:
 *  - Register read/write
 *  - Multi-byte transfers
 *  - Optional debug output
 *  - Device handle caching (ESP-IDF)
 */
class I2CBus {
public:
#ifdef ARDUINO

    bool begin(TwoWire* wire = &Wire, uint8_t sda = 38, uint8_t scl = 39, uint32_t freq = 400000);

#else

    bool begin(i2c_port_num_t port = I2C_NUM_0, int sda = 38, int scl = 39, uint32_t freq = 400000);

#endif

    bool writeReg(uint8_t addr, uint8_t reg, uint8_t value);

    bool writeBytes(uint8_t addr, uint8_t reg, const uint8_t* data, size_t len);

    bool readBytes(uint8_t addr, uint8_t reg, uint8_t* data, size_t len);

private:
#ifdef ARDUINO
    TwoWire* _wire = nullptr;
#else
    i2c_master_bus_handle_t _bus_handle = nullptr;
    uint32_t _freq                      = 400000;

    struct DevEntry {
        uint8_t addr;
        i2c_master_dev_handle_t handle;
    };

    DevEntry _dev_cache[I2C_DEV_CACHE_MAX] = {};
    uint8_t _dev_count                     = 0;

    i2c_master_dev_handle_t getDevHandle(uint8_t addr);
#endif
};

#endif  // I2C_BUS_H