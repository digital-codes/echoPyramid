/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _M5_ECHO_PYRAMID_H_
#define _M5_ECHO_PYRAMID_H_

// ============================================================
//  Platform Detection
// ============================================================
#ifdef ARDUINO
#include <Arduino.h>
#include <Wire.h>
#endif
#include "esp_idf_version.h"

#if ESP_IDF_VERSION_MAJOR >= 5
#include "driver/i2s_std.h"
#else
#include "driver/i2s.h"
#endif

#include "I2CBus.h"
#include "SI5351.h"
#include "ES7210.h"
#include "ES8311.h"
#include "STM32Ctrl.h"
#include "AW87559.h"

/**
 * @brief High-level audio system wrapper for M5EchoPyramid.
 *
 * This class integrates:
 *  - SI5351 clock generator
 *  - ES7210 ADC (Microphone input)
 *  - ES8311 Audio Codec (DAC + ADC)
 *  - STM32 controller (Touch + RGB)
 *  - AW87559 Power Amplifier
 *
 * It provides a unified interface for:
 *  - Audio input (microphone capture)
 *  - Audio output (speaker playback)
 *  - Peripheral access
 *
 * Compatible with:
 *  - Arduino framework
 *  - ESP-IDF v5.x
 */
class M5EchoPyramid {
public:
    /**
     * @brief Constructor.
     *
     * Initializes internal device objects.
     */
    M5EchoPyramid();

#ifdef ARDUINO

    /**
     * @brief Initialize system (Arduino version).
     *
     * @param wire        I2C bus object (default: Wire)
     * @param sda         I2C SDA pin
     * @param scl         I2C SCL pin
     * @param bclk        I2S bit clock pin
     * @param lrck        I2S word select (LRCK) pin
     * @param dout        I2S data output pin (to DAC)
     * @param din         I2S data input pin (from ADC)
     * @param sample_rate Audio sample rate (default: 44100 Hz)
     *
     * @return true if initialization successful
     */
    bool begin(TwoWire* wire = &Wire, uint8_t sda = 38, uint8_t scl = 39, int bclk = 6, int lrck = 8, int dout = 5,
               int din = 7, uint32_t sample_rate = 44100);

#else

    /**
     * @brief Initialize system (ESP-IDF version).
     *
     * @param i2c_port    I2C port number
     * @param sda         I2C SDA pin
     * @param scl         I2C SCL pin
     * @param bclk        I2S bit clock pin
     * @param lrck        I2S word select pin
     * @param dout        I2S data output pin
     * @param din         I2S data input pin
     * @param sample_rate Audio sample rate
     *
     * @return true if initialization successful
     */
    bool begin(i2c_port_num_t i2c_port = I2C_NUM_0, int sda = 38, int scl = 39, int bclk = 6, int lrck = 8,
               int dout = 5, int din = 7, uint32_t sample_rate = 44100);
#endif

    /**
     * @brief Read audio samples from microphone.
     *
     * @param mic  Buffer to store microphone samples
     * @param ref  Buffer to store reference channel samples
     * @param frames Number of audio frames to read
     *
     * This function reads from I2S RX channel.
     */
    void read(int16_t* mic, int16_t* ref, int frames);

    /**
     * @brief Write audio samples to speaker.
     *
     * @param data   Audio sample buffer (16-bit PCM)
     * @param frames Number of audio frames to write
     *
     * This function writes to I2S TX channel.
     */
    void write(int16_t* data, int frames);

    /* ================= Peripheral Access ================= */

    /**
     * @brief Get ES8311 codec reference.
     */
    ES8311& codec();

    /**
     * @brief Get ES7210 ADC reference.
     */
    ES7210& adc();

    /**
     * @brief Get STM32 controller reference.
     */
    STM32Ctrl& ctrl();

    /**
     * @brief Get AW87559 power amplifier reference.
     */
    AW87559& pa();

private:
#ifdef ARDUINO
    TwoWire* _wire = &Wire;  ///< I2C bus pointer (Arduino)
#endif

    I2CBus _bus;       ///< I2C abstraction
    SI5351 _si5351;    ///< Clock generator
    ES7210 _es7210;    ///< ADC (microphone)
    ES8311 _es8311;    ///< Codec (DAC + ADC)
    STM32Ctrl _stm32;  ///< Touch & RGB controller
    AW87559 _pa;       ///< Power amplifier

    /**
     * @brief Initialize I2S peripheral.
     *
     * @param bclk        Bit clock pin
     * @param lrck        Word select pin
     * @param dout        Data output pin
     * @param din         Data input pin
     * @param sample_rate Audio sample rate
     */
    void initI2S(int bclk, int lrck, int dout, int din, uint32_t sample_rate);

#if ESP_IDF_VERSION_MAJOR >= 5
    i2s_chan_handle_t _tx_chan = nullptr;
    i2s_chan_handle_t _rx_chan = nullptr;
#endif
};

#endif