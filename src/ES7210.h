/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _ES7210_H_
#define _ES7210_H_

#include "I2CBus.h"

/* ============================================================
 *  I2C Address & Register Definitions
 * ============================================================
 */

/**
 * @brief Default I2C address (AD1=0, AD0=0).
 */
#define ES7210_I2C_ADDR (0x40)

/* ===== Register Map ===== */
#define ES7210_RESET_REG00           (0x00) /**< Reset control */
#define ES7210_CLOCK_OFF_REG01       (0x01) /**< ADC clock disable */
#define ES7210_MAINCLK_REG02         (0x02) /**< ADC clock divider */
#define ES7210_MASTER_CLK_REG03      (0x03) /**< MCLK source / SCLK divider */
#define ES7210_LRCK_DIVH_REG04       (0x04)
#define ES7210_LRCK_DIVL_REG05       (0x05)
#define ES7210_POWER_DOWN_REG06      (0x06)
#define ES7210_OSR_REG07             (0x07)
#define ES7210_MODE_CONFIG_REG08     (0x08) /**< Master/slave & channel config */
#define ES7210_TIME_CONTROL0_REG09   (0x09)
#define ES7210_TIME_CONTROL1_REG0A   (0x0A)
#define ES7210_SDP_INTERFACE1_REG11  (0x11) /**< Audio format config */
#define ES7210_SDP_INTERFACE2_REG12  (0x12)
#define ES7210_ADC_AUTOMUTE_REG13    (0x13)
#define ES7210_ADC34_MUTERANGE_REG14 (0x14)
#define ES7210_ADC34_HPF2_REG20      (0x20)
#define ES7210_ADC34_HPF1_REG21      (0x21)
#define ES7210_ADC12_HPF1_REG22      (0x22)
#define ES7210_ADC12_HPF2_REG23      (0x23)
#define ES7210_ANALOG_REG40          (0x40)
#define ES7210_MIC12_BIAS_REG41      (0x41)
#define ES7210_MIC34_BIAS_REG42      (0x42)
#define ES7210_MIC1_GAIN_REG43       (0x43)
#define ES7210_MIC2_GAIN_REG44       (0x44)
#define ES7210_MIC3_GAIN_REG45       (0x45)
#define ES7210_MIC4_GAIN_REG46       (0x46)
#define ES7210_MIC1_POWER_REG47      (0x47)
#define ES7210_MIC2_POWER_REG48      (0x48)
#define ES7210_MIC3_POWER_REG49      (0x49)
#define ES7210_MIC4_POWER_REG4A      (0x4A)
#define ES7210_MIC12_POWER_REG4B     (0x4B)
#define ES7210_MIC34_POWER_REG4C     (0x4C)

/* ============================================================
 *  Enumerations
 * ============================================================
 */

/**
 * @brief I2C address selection via AD1/AD0 pins.
 */
typedef enum {
    ES7210_AD1_AD0_00 = 0x80,
    ES7210_AD1_AD0_01 = 0x82,
    ES7210_AD1_AD0_10 = 0x84,
    ES7210_AD1_AD0_11 = 0x86
} es7210_address_t;

/**
 * @brief Microphone input channel mask.
 */
typedef enum {
    ES7210_INPUT_MIC1 = 0x01,
    ES7210_INPUT_MIC2 = 0x02,
    ES7210_INPUT_MIC3 = 0x04,
    ES7210_INPUT_MIC4 = 0x08
} es7210_input_mics_t;

/**
 * @brief Microphone PGA gain levels.
 *
 * Gain range: 0 dB ~ 37.5 dB
 */
typedef enum {
    GAIN_0DB = 0,
    GAIN_3DB,
    GAIN_6DB,
    GAIN_9DB,
    GAIN_12DB,
    GAIN_15DB,
    GAIN_18DB,
    GAIN_21DB,
    GAIN_24DB,
    GAIN_27DB,
    GAIN_30DB,
    GAIN_33DB,
    GAIN_34_5DB,
    GAIN_36DB,
    GAIN_37_5DB,
} es7210_gain_value_t;

/* ============================================================
 *  ES7210 Class Definition
 * ============================================================
 */

/**
 * @brief Driver for ES7210 4-Channel ADC Codec.
 *
 * Features:
 *  - 4-channel microphone input
 *  - Programmable PGA gain
 *  - MCLK-based clock configuration
 *  - Master/Slave mode support
 *  - High-pass filter support
 *
 * Typical Use Case:
 *  - Multi-microphone array
 *  - Beamforming systems
 *  - Echo cancellation reference capture
 */
class ES7210 {
public:
    /**
     * @brief Constructor.
     *
     * @param bus  Reference to I2CBus instance
     * @param addr I2C device address (default 0x40)
     */
    ES7210(I2CBus& bus, uint8_t addr = ES7210_I2C_ADDR);

    /**
     * @brief Initialize ES7210 ADC.
     *
     * @param mclk_hz    External master clock frequency (Hz)
     * @param sample_rate Target sample rate (default 44100)
     * @param mic_mask   Bitmask of enabled microphones
     *
     * @return true if initialization successful
     */
    bool begin(uint32_t mclk_hz, uint32_t sample_rate = 44100,
               uint8_t mic_mask = ES7210_INPUT_MIC1 | ES7210_INPUT_MIC3);

    /**
     * @brief Set gain for a specific microphone channel.
     *
     * @param mic  Microphone index (1~4)
     * @param gain Gain level (0~37.5 dB)
     *
     * @return true if successful
     */
    bool setMicGain(uint8_t mic, es7210_gain_value_t gain);

    /**
     * @brief Enable or disable ADC mute.
     *
     * @param enable true = mute, false = unmute
     *
     * @return true if successful
     */
    bool mute(bool enable);

    /**
     * @brief Power on/off specific microphone channel.
     *
     * @param mic    Microphone index (1~4)
     * @param enable true = power on, false = power off
     *
     * @return true if successful
     */
    bool powerMic(uint8_t mic, bool enable);

private:
    I2CBus& _bus;   ///< I2C bus reference
    uint8_t _addr;  ///< I2C device address

    /**
     * @brief Write a single 8-bit register of the ES7210.
     *
     * Sends a register address followed by an 8-bit data value over I2C.
     *
     * @param reg Register address to write.
     * @param val 8-bit value to be written to the register.
     *
     * @return true  Register write successful.
     * @return false I2C communication failed.
     *
     * @note This is a low-level helper function used internally by the driver.
     *       It does not perform parameter validation.
     */
    bool writeReg(uint8_t reg, uint8_t val);

    /**
     * @brief Read a single 8-bit register of the ES7210.
     *
     * Performs an I2C register read operation and stores the retrieved value
     * in the provided reference parameter.
     *
     * @param reg Register address to read.
     * @param val Reference variable to store the read 8-bit value.
     *
     * @return true  Register read successful.
     * @return false I2C communication failed.
     *
     * @note This is a low-level helper function used internally by the driver.
     *       It does not perform parameter validation.
     */
    bool readReg(uint8_t reg, uint8_t& val);
};

#endif  // ES7210_H