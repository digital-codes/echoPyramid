/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _ES8311_H_
#define _ES8311_H_

#include "I2CBus.h"

/**
 * @brief Default I2C address for ES8311 codec.
 */
#define ES8311_I2C_ADDR (0x18)

/* ============================================================
 *  Register Definitions (from ES8311 datasheet)
 * ============================================================
 */

// ===== System & Clock Registers =====
#define ES8311_RESET_REG00       (0x00)
#define ES8311_CLK_MANAGER_REG01 (0x01)
#define ES8311_CLK_MANAGER_REG02 (0x02)
#define ES8311_CLK_MANAGER_REG03 (0x03)
#define ES8311_CLK_MANAGER_REG04 (0x04)
#define ES8311_CLK_MANAGER_REG05 (0x05)
#define ES8311_CLK_MANAGER_REG06 (0x06)
#define ES8311_CLK_MANAGER_REG07 (0x07)
#define ES8311_CLK_MANAGER_REG08 (0x08)

// ===== Digital Audio Interface =====
#define ES8311_SDPIN_REG09  (0x09)
#define ES8311_SDPOUT_REG0A (0x0A)

// ===== System Control =====
#define ES8311_SYSTEM_REG0B (0x0B)
#define ES8311_SYSTEM_REG0C (0x0C)
#define ES8311_SYSTEM_REG0D (0x0D)
#define ES8311_SYSTEM_REG0E (0x0E)
#define ES8311_SYSTEM_REG0F (0x0F)
#define ES8311_SYSTEM_REG10 (0x10)
#define ES8311_SYSTEM_REG11 (0x11)
#define ES8311_SYSTEM_REG12 (0x12)
#define ES8311_SYSTEM_REG13 (0x13)
#define ES8311_SYSTEM_REG14 (0x14)

// ===== ADC Registers =====
#define ES8311_ADC_REG15 (0x15)
#define ES8311_ADC_REG16 (0x16)
#define ES8311_ADC_REG17 (0x17)
#define ES8311_ADC_REG18 (0x18)
#define ES8311_ADC_REG19 (0x19)
#define ES8311_ADC_REG1A (0x1A)
#define ES8311_ADC_REG1B (0x1B)
#define ES8311_ADC_REG1C (0x1C)

// ===== DAC Registers =====
#define ES8311_DAC_REG31 (0x31)
#define ES8311_DAC_REG32 (0x32)
#define ES8311_DAC_REG33 (0x33)
#define ES8311_DAC_REG34 (0x34)
#define ES8311_DAC_REG35 (0x35)
#define ES8311_DAC_REG37 (0x37)

// ===== GPIO & Chip ID =====
#define ES8311_GPIO_REG44  (0x44)
#define ES8311_GP_REG45    (0x45)
#define ES8311_CHD1_REGFD  (0xFD)
#define ES8311_CHD2_REGFE  (0xFE)
#define ES8311_CHVER_REGFF (0xFF)

/* ============================================================
 *  Clock Coefficient Table Structure
 * ============================================================
 */

/**
 * @brief Clock coefficient configuration entry.
 *
 * Defines all divider and oversampling parameters required
 * to configure ES8311 internal clocks for a given:
 *
 *      MCLK frequency + sample rate
 */
struct es8311_coeff_div {
    uint32_t mclk;  ///< Master clock frequency (Hz)
    uint32_t rate;  ///< Audio sample rate (Hz)

    uint8_t pre_div;    ///< REG02[7:5]
    uint8_t pre_multi;  ///< REG02[4:3]

    uint8_t adc_div;  ///< REG05[7:4]
    uint8_t dac_div;  ///< REG05[3:0]

    uint8_t fs_mode;  ///< REG03[7:6]
    uint8_t lrck_h;   ///< REG07[5:0]
    uint8_t lrck_l;   ///< REG08[7:0]

    uint8_t bclk_div;  ///< REG06[4:0]

    uint8_t adc_osr;  ///< REG03[5:0]
    uint8_t dac_osr;  ///< REG04[7:0]
};

/* ============================================================
 *  ES8311 Class Definition
 * ============================================================
 */

/**
 * @brief Driver for ES8311 Audio Codec.
 *
 * Features:
 *  - I2C register abstraction
 *  - Clock configuration (MCLK + sample rate)
 *  - Volume control
 *  - Mute control
 *  - Microphone gain adjustment
 *
 * Typical clocking:
 *      MCLK = 256 × sample_rate
 * Example:
 *      44100 Hz → 11289600 Hz
 */
class ES8311 {
public:
    /**
     * @brief Constructor.
     *
     * @param bus  Reference to I2CBus instance
     * @param addr I2C device address (default 0x18)
     */
    ES8311(I2CBus& bus, uint8_t addr = ES8311_I2C_ADDR);

    /**
     * @brief Initialize codec.
     *
     * @param mclk_hz     External master clock frequency (Hz)
     * @param sample_rate Target audio sample rate
     *
     * @return true if initialization successful
     */
    bool begin(uint32_t mclk_hz, uint32_t sample_rate = 44100);

    bool setVolume(uint8_t volume);
    bool getVolume(uint8_t& volume);
    bool mute(bool enable);
    bool setMicGain(uint8_t gain);

private:
    I2CBus& _bus;
    uint8_t _addr;

    bool writeReg(uint8_t reg, uint8_t val);
    bool readReg(uint8_t reg, uint8_t& val);

    bool sampleFrequencyConfig(uint32_t mclk_hz, uint32_t sample_rate);
    bool clockConfig(uint32_t mclk_hz);

    const es8311_coeff_div* getCoeff(uint32_t mclk, uint32_t rate);
};

#endif  // ES8311_H