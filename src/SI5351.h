/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _SI5351_DRIVER_H_
#define _SI5351_DRIVER_H_

#include "I2CBus.h"

/* ================= Device Address ================= */

// Default I2C address of SI5351
#define SI5351_I2C_ADDR (0x60)

/**
 * @brief Driver for SI5351 programmable clock generator.
 *
 * This class provides basic configuration for:
 *  - Audio sample rate clock generation
 *  - Master clock (MCLK) output
 *
 * Typical usage:
 *  - Generate 4.096 MHz for 16 kHz audio
 *  - Generate 11.2896 MHz for 44.1 kHz audio
 *  - Generate 12.288 MHz for 48 kHz audio
 *
 * Communication is performed over I2C.
 */
class SI5351 {
public:
    /**
     * @brief Constructor
     *
     * @param bus Reference to I2C bus object
     * @param addr I2C address of SI5351 (default: 0x60)
     */
    SI5351(I2CBus& bus, uint8_t addr = SI5351_I2C_ADDR);

    /**
     * @brief Initialize SI5351 device.
     *
     * This function should:
     *  - Configure internal PLL
     *  - Setup default output clock
     *  - Enable required clock outputs
     *
     * @return true if device responds correctly
     */
    bool begin();

    /**
     * @brief Configure audio sample rate clock.
     *
     * Supported typical sample rates:
     *  - 16000 Hz
     *  - 44100 Hz
     *  - 48000 Hz
     *
     * Internally this sets appropriate PLL and multisynth dividers.
     *
     * @param sample_rate Desired audio sample rate in Hz
     * @return true if configuration successful
     */
    bool setSampleRate(uint32_t sample_rate);

    /**
     * @brief Set master clock (MCLK) frequency directly.
     *
     * Typical MCLK values:
     *  - 4,096,000 Hz   (for 16 kHz)
     *  - 11,289,600 Hz  (for 44.1 kHz)
     *  - 12,288,000 Hz  (for 48 kHz)
     *
     * @param freq Desired MCLK frequency in Hz
     * @return true if configuration successful
     */
    bool setMCLK(uint32_t freq);

private:
    I2CBus& _bus;   ///< Reference to I2C bus
    uint8_t _addr;  ///< I2C address of SI5351

    /**
     * @brief Write single register.
     *
     * @param reg Register address
     * @param val Register value
     * @return true if write successful
     */
    bool writeReg(uint8_t reg, uint8_t val);

    /**
     * @brief Write multiple consecutive registers.
     *
     * @param reg Start register address
     * @param data Pointer to data buffer
     * @param len Number of bytes to write
     * @return true if write successful
     */
    bool writeBulk(uint8_t reg, uint8_t* data, uint8_t len);

    /**
     * @brief Configure PLL frequency.
     *
     * @param pll_freq Target PLL frequency in Hz
     * @param ms_div Multisynth divider value
     * @return true if configuration successful
     */
    bool setPLL(uint32_t pll_freq, uint32_t ms_div);

    /**
     * @brief Configure multisynth divider.
     *
     * @param div Divider value
     * @return true if configuration successful
     */
    bool setupMultisynth(uint32_t div);

    /**
     * @brief Reset PLL to apply new configuration.
     */
    void resetPLL();
};

#endif